# ExpressDesigner — Undo/Redo Architecture
## Generated: June 2026

---

## 1. Overview

The undo/redo subsystem implements the **Command Pattern** with a dual-stack history. Every mutation to the project model flows through `CommandHistory::push()`, which executes the command immediately and records it. Undo/redo reverses or replays the command by calling `undo()` or `execute()` respectively on the stored command object.

The system is **not** a snapshot-based approach — each command stores only the minimum data needed to revert the change (deltas, old values, old control points). After undo/redo, `MainWindow` calls `recalculateAll()`, which removes all current result objects and re-executes every operation in creation order. This ensures all downstream results are recomputed from the restored state.

---

## 2. Core Classes

### 2.1 `Command` (abstract base)
**File:** `src/core/CommandHistory.h`

```cpp
class Command {
public:
    explicit Command(const QString& description = {});
    virtual ~Command();
    virtual bool execute(Project* project) = 0;  // perform the action
    virtual bool undo(Project* project) = 0;      // revert the action
    QString description() const;
    QDateTime timestamp() const;
protected:
    QString m_description;
    QDateTime m_timestamp;
};
```

**Key design decisions:**
- Every command carries a human-readable `description()` and a `timestamp()` for audit/history display.
- `execute()` and `undo()` receive a `Project*` — the command itself is stateless regarding the project; it operates on whatever project is passed.
- No `modifiedObjectName()` method — after undo/redo, `recalculateAll()` re-executes all operations unconditionally. There is no need to identify a specific "affected object."

### 2.2 `CommandHistory`
**File:** `src/core/CommandHistory.h` / `src/core/CommandHistory.cpp`

```cpp
class CommandHistory : public QObject {
    static constexpr int kMaxStack = 200;
    std::vector<std::unique_ptr<Command>> m_undoStack;
    std::vector<std::unique_ptr<Command>> m_redoStack;
};
```

**Stack management rules:**
- `push()` calls `cmd->execute(project)` first. If execute succeeds, the command moves to `m_undoStack` and `m_redoStack` is **cleared**. This is standard command-pattern behavior: new actions invalidate the redo branch.
- The undo stack is capped at **200 entries** (FIFO eviction from the front when overflow occurs).
- `undo()` pops from `m_undoStack`, calls `cmd->undo(project)`, and pushes to `m_redoStack`. If `undo()` fails, the command is re-executed (`cmd->execute()`) and pushed back to the undo stack (rollback on failure).
- `redo()` pops from `m_redoStack`, calls `cmd->execute(project)`, and pushes to `m_undoStack`. Same failure rollback pattern.
- `stackChanged()` signal is emitted on every mutation of either stack. Connected to `MainWindow::updateUndoRedoActions()`.

### 2.3 Undo/Redo + Recalculation Bridge

In the simplified architecture, undo and redo simply call `recalculateAll()` after restoring/redoing the state. There is no surgical recalculation or targeted object identification:

```
MainWindow::onUndo()
  → m_cmdHistory->undo(project)      // restore previous state
  → recalculateAll()                 // re-execute ALL operations
  → refreshChart()

MainWindow::onRedo()
  → m_cmdHistory->redo(project)      // re-apply state
  → recalculateAll()                 // re-execute ALL operations
  → refreshChart()
```

This is simpler than the previous approach which used `modifiedObjectName()` to surgically recalculate only the affected object's dependents. By always re-executing everything, correctness is guaranteed with no dependency tracking needed at the CommandHistory level.

---

## 3. Concrete Commands

| Command Class | Trigger | Data Stored | Execute | Undo |
|---|---|---|---|---|
| `AddObjectCommand` | Insert dialog | `CustomObject*`, `m_wasResult` | Calls `project->addDataObject()` or `addResultObject()` | Calls `project->removeDataObject()` or `removeResultObject()` |
| `DeleteObjectCommand` | Delete action | `CustomObject*`, `m_isResult`, `m_index` | `removeDataObject()` or `removeResultObject()`, saves index | `addDataObject()` or `addResultObject()` |
| `ModifyObjectCommand` | Property editor change | `CustomObject*`, property name, old/new `QVariant` | Applies new value via `setName()`/`setRefractiveIndex()`/etc. | Restores old value |
| `ModifyControlPointsCommand` | Point editor | `CustomObject*`, old & new `QVector<QPointF>` | `m_obj->setControlPoints(m_newPoints)` | `m_obj->setControlPoints(m_oldPoints)` |
| `ExecuteOperationCommand` | CalcOval / PropagateWF | `CustomOperation*`, `m_resultObj`, `m_wasAdded` | Calls `op->execute()`, tracks result object | Removes resultObject and operation |
| `RotateObjectCommand` | Rotate dialog | `CustomObject*`, `m_oldPoints`, pivot, degrees | Computes rotation matrix, applies new points | Restores `m_oldPoints` |
| `TranslateObjectCommand` | Translate dialog | `CustomObject*`, `m_delta` (QPointF) | Adds delta to all control points | Subtracts delta from all control points |

**Note:** All commands are self-contained — they only modify the project model (objects, operations, control points). After undo or redo, `recalculateAll()` handles downstream result recomputation.

---

## 4. Algorithm: `CommandHistory::push()`

```
push(unique_ptr<Command> cmd, Project* project):
    1. if cmd is null → return false
    2. if !cmd->execute(project) → return false   // pre-flight execution
    3. m_redoStack.clear()                         // invalidate redo branch
    4. m_undoStack.push_back(std::move(cmd))
    5. while m_undoStack.size() > kMaxStack:
         m_undoStack.erase(m_undoStack.begin())    // evict oldest
    6. emit stackChanged()
    7. return true
```

**Critical property:** The command is executed before being pushed. If execute fails, the command is discarded and nothing is recorded. This ensures the undo stack only contains commands that actually produced a valid state.

---

## 5. Algorithm: `CommandHistory::undo()`

```
undo(Project* project):
    1. if m_undoStack is empty → return false
    2. cmd = std::move(m_undoStack.back())
    3. m_undoStack.pop_back()
    4. if !cmd->undo(project):
         // FAIL: rollback — re-execute the command to restore state
         cmd->execute(project)
         m_undoStack.push_back(std::move(cmd))
         return false
    5. m_redoStack.push_back(std::move(cmd))
    6. emit stackChanged()
    7. return true
```

**Failure handling:** If `undo()` returns false (e.g., the object no longer exists, the project is in an unexpected state), the command is re-executed to restore the state and pushed back to the undo stack. The user's state is preserved.

---

## 6. Algorithm: `CommandHistory::redo()`

```
redo(Project* project):
    1. if m_redoStack is empty → return false
    2. cmd = std::move(m_redoStack.back())
    3. m_redoStack.pop_back()
    4. if !cmd->execute(project):
         m_redoStack.push_back(std::move(cmd))
         return false
    5. m_undoStack.push_back(std::move(cmd))
    6. emit stackChanged()
    7. return true
```

Symmetric to undo. On execute failure, the command stays in the redo stack.

---

## 7. Full Flow: User rotates S1, then presses Undo

### Step 1: Rotate Object (via `onRotateObject()`)
```
MainWindow::onRotateObject()
  → RotateObjectCommand cmd(obj=S1, degrees=45, pivot=MidPoint)
  → m_cmdHistory->push(std::move(cmd), project)
      → cmd.execute(project):
          - save m_oldPoints = current control points of S1
          - compute new points by rotating around midpoint
          - obj->setControlPoints(newPoints)
          - return true
      → push to m_undoStack
      → clear m_redoStack
      → emit stackChanged()
  → recalculateAll()                 // re-execute all operations
  → refreshChart()
```

### Step 2: Undo (via `onUndo()`, Ctrl+Z)
```
MainWindow::onUndo()
  → m_cmdHistory->undo(project)
      → pop RotateObjectCommand from m_undoStack
      → cmd.undo(project):
          - obj->setControlPoints(m_oldPoints)  // restore pre-rotation state
          - return true
      → push to m_redoStack
      → emit stackChanged()
  → recalculateAll()                 // re-execute all operations from restored state
  → refreshChart()
  → updateStatusBar()
  → setModified(true)
```

### Step 3: Redo (via `onRedo()`, Ctrl+Y)
```
MainWindow::onRedo()
  → m_cmdHistory->redo(project)
      → pop RotateObjectCommand from m_redoStack
      → cmd.execute(project):
          - reads current points of S1 (pre-rotation after undo)
          - saves them as m_oldPoints
          - computes rotation again using m_degrees and m_pivot
          - obj->setControlPoints(newPoints)
          - return true
      → push to m_undoStack
      → emit stackChanged()
  → recalculateAll()                 // re-execute all operations
  → refreshChart()
  → updateStatusBar()
  → setModified(true)
```

**Important subtlety for RotateObjectCommand::execute():** On redo, `execute()` reads the current points of S1 (which are the pre-rotation points after undo), saves them as `m_oldPoints`, computes the rotation again from scratch using `m_degrees` and `m_pivot`, and applies the result. The command is **idempotent** — execute always produces the same rotation regardless of whether it's the first call or a redo.

---

## 8. Recalculation After Undo/Redo

After every undo or redo operation, `MainWindow` calls `recalculateAll()`:

```
recalculateAll():
    1. Block project signals (QSignalBlocker)
    2. Remove ALL current result objects
    3. Re-execute every operation in m_operations (insertion order)
    4. Rebuild DependencyGraph (for UI dialogs only)
    5. Refresh chart, status bar, mark as modified
```

This replaces the previous surgical approach (`recalcDependents(obj)`) that used `DependencyGraph` inside the loop and required `modifiedObjectName()` to identify which object changed. The new approach is simpler and guarantees correctness by recomputing everything.

See `RecalculateArchitecture.md` for the full recalculation design.

---

## 9. Thread Safety & Signal Handling

- **Single-threaded:** All command execution, undo, and redo happen on the Qt main thread. No synchronization primitives needed.
- **Signal blocking:** `CommandHistory::push()`, `undo()`, and `redo()` do **not** block project signals. It's the caller's responsibility (`MainWindow::recalculateAll()`) to block signals during recalculation via `QSignalBlocker`.

---

## 10. Edge Cases & Design Decisions

1. **Execute-before-push:** The command is executed before being pushed. If it fails, nothing is recorded. This means the UI state should already be prepared for the change before calling push.

2. **Undo/Redo with deleted objects:** Commands store raw `CustomObject*` pointers. If the object was deleted by another action between undo and redo, the undo/redo would return false (null check on `m_obj`). The failure would be caught and the command would be rolled back.

3. **Stack cap at 200:** Prevents unbounded memory growth. Oldest commands are silently evicted. The undo menu reflects this via `canUndo()`/`canRedo()`.

4. **Redo stack cleared on new command:** Standard behavior — a new mutation creates a new timeline branch, invalidating any previous redo history.

5. **Description localization:** `undoText()` and `redoText()` use `tr()` for UI localization (e.g., "Undo Rotate MyObject").

6. **Ownership:** Commands are owned exclusively by the stacks via `std::unique_ptr`. No raw pointer sharing. Moving between stacks transfers ownership.

---

## 11. File Index

| File | Purpose |
|---|---|
| `src/core/CommandHistory.h` | `Command` base class, all concrete commands, `CommandHistory` class |
| `src/core/CommandHistory.cpp` | Full implementation of all commands and stack logic |
| `src/ui/MainWindow.h` | `m_cmdHistory` member, `onUndo()`, `onRedo()`, `updateUndoRedoActions()`, `recalculateAll()` |
| `src/ui/MainWindow.cpp` | `onUndo()`, `onRedo()`, `updateUndoRedoActions()`, `recalculateAll()` |