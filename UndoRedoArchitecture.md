# ExpressDesigner — Undo/Redo Architecture
## Generated: June 2026

---

## 1. Overview

The undo/redo subsystem implements the **Command Pattern** with a dual-stack history. Every mutation to the project model flows through `CommandHistory::push()`, which executes the command immediately and records it. Undo/redo reverses or replays the command by calling `undo()` or `execute()` respectively on the stored command object.

The system is **not** a snapshot-based approach — each command stores only the minimum data needed to revert the change (deltas, old values, old control points). On undo/redo, the `DependencyGraph` is rebuilt and `recalcDependents()` is called as a side-effect to propagate changes through the operation chain.

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
    virtual QString modifiedObjectName() const { return {}; }  // for recalc routing
    QString description() const;
    QDateTime timestamp() const;
protected:
    QString m_description;
    QDateTime m_timestamp;
};
```

**Key design decisions:**
- Every command carries a human-readable `description()` and a `timestamp()` for audit/history display.
- `modifiedObjectName()` returns the name of the object directly affected. This is used after undo/redo to identify which object needs recalculation of its dependents. Only `RotateObjectCommand` and `TranslateObjectCommand` override this — all others return empty string (meaning no targeted recalc needed after undo/redo of those command types).
- `execute()` and `undo()` receive a `Project*` — the command itself is stateless regarding the project; it operates on whatever project is passed.

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

### 2.3 `lastUndoneModifiedObjectName()` / `lastRedoneModifiedObjectName()`

These are the bridge between undo/redo and the recalculation system:

- **`lastUndoneModifiedObjectName()`** reads the `modifiedObjectName()` from the top of the **redo** stack (the command that was just undone). Conceptually: "which object did we just revert?"
- **`lastRedoneModifiedObjectName()`** reads from the top of the **undo** stack (the command that was just redone). Conceptually: "which object did we just re-apply?"

Example: If the user rotates object "S1" and then presses Undo:
1. `RotateObjectCommand` moves from undo stack to redo stack
2. `lastUndoneModifiedObjectName()` returns `"S1"`
3. `MainWindow::onUndo()` calls `recalcDependents(S1)` to cascade-recalculate

---

## 3. Concrete Commands

| Command Class | Trigger | Data Stored | Execute | Undo | modifiedObjectName |
|---|---|---|---|---|---|
| `AddObjectCommand` | Insert dialog | `CustomObject*`, `m_wasResult` | Calls `project->addDataObject()` or `addResultObject()` | Calls `project->removeDataObject()` or `removeResultObject()` | Empty |
| `DeleteObjectCommand` | Delete action | `CustomObject*`, `m_isResult`, `m_index` | `removeDataObject()` or `removeResultObject()`, saves index | `addDataObject()` or `addResultObject()` | Empty |
| `ModifyObjectCommand` | Property editor change | `CustomObject*`, property name, old/new `QVariant` | Applies new value via `setName()`/`setRefractiveIndex()`/etc. | Restores old value | Empty |
| `ModifyControlPointsCommand` | Point editor | `CustomObject*`, old & new `QVector<QPointF>` | `m_obj->setControlPoints(m_newPoints)` | `m_obj->setControlPoints(m_oldPoints)` | Empty |
| `ExecuteOperationCommand` | CalcOval / PropagateWF | `CustomOperation*`, `m_resultObj`, `m_wasAdded` | Calls `op->execute()`, tracks result object | Removes resultObject and operation | Empty |
| `RotateObjectCommand` | Rotate dialog | `CustomObject*`, `m_oldPoints`, pivot, degrees | Computes rotation matrix, applies new points | Restores `m_oldPoints` | **Name of rotated object** |
| `TranslateObjectCommand` | Translate dialog | `CustomObject*`, `m_delta` (QPointF) | Adds delta to all control points | Subtracts delta from all control points | **Name of translated object** |

### 3.1 Why only Rotate/Translate return modifiedObjectName?

Rotate and Translate modify an object's geometry (control points) without changing its existence or operations. The object keeps the same name and identity, so dependents can be recalculated by name. 

For `ExecuteOperationCommand` (CalcOval, PropagateWF), undo/redo adds/removes the operation and its result — the dependency graph gets fully rebuilt instead.

For `ModifyControlPointsCommand`, the current implementation returns empty — recalc is triggered directly by the caller in `MainWindow` (e.g., `onEditPoints()`) rather than through the undo/redo path.

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
  → recalcDependents(S1)              // side-effect: recalculate downstream
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
  → m_depGraph->rebuildFromProject(project)
  → objName = m_cmdHistory->lastUndoneModifiedObjectName()
      → reads m_redoStack.back()->modifiedObjectName() → "S1"
  → modifiedObj = project->findObject("S1")
  → recalcDependents(modifiedObj)     // S1 was restored, recalc its dependents
  → refreshChart()
```

### Step 3: Redo (via `onRedo()`, Ctrl+Y)
```
MainWindow::onRedo()
  → m_cmdHistory->redo(project)
      → pop RotateObjectCommand from m_redoStack
      → cmd.execute(project):
          - apply rotation again (m_oldPoints were restored in step 2,
            m_newPoints were computed in step 1 and stored in the command)
          - wait — this re-executes from the current state
          - Actually: execute() recomputes the rotation from current points
            and saves them as m_oldPoints again. The rotation is re-applied.
          - return true
      → push to m_undoStack
      → emit stackChanged()
  → m_depGraph->rebuildFromProject(project)
  → objName = m_cmdHistory->lastRedoneModifiedObjectName() → "S1"
  → recalcDependents(modifiedObj)
  → refreshChart()
```

**Important subtlety for RotateObjectCommand::execute():** On redo, `execute()` reads the current points of S1 (which are the pre-rotation points after undo), saves them as `m_oldPoints`, computes the rotation again from scratch using `m_degrees` and `m_pivot`, and applies the result. This means the command is **idempotent** — execute always produces the same rotation regardless of whether it's the first call or a redo.

---

## 8. ModifiedObjectName + Recalc Bridge

The `modifiedObjectName()` mechanism is the key link between undo/redo and recalculation:

```
CommandHistory storage:
  undoStack: [cmd_N, cmd_N-1, ..., cmd_0]   (most recent at back)
  redoStack: [undone_cmd_0, ..., undone_cmd_M]

After undo():
  - lastUndoneModifiedObjectName() reads redoStack.back()->modifiedObjectName()
  - returns the name of the object that was just restored

After redo():
  - lastRedoneModifiedObjectName() reads undoStack.back()->modifiedObjectName()
  - returns the name of the object that was just re-applied
```

Commands without `modifiedObjectName` (AddObject, DeleteObject, ModifyObject, ModifyControlPoints, ExecuteOperation) return empty string. In those cases, `MainWindow` skips the targeted recalc call — the dependency graph rebuild alone is sufficient.

---

## 9. Thread Safety & Signal Handling

- **Single-threaded:** All command execution, undo, and redo happen on the Qt main thread. No synchronization primitives needed.
- **Signal blocking:** `CommandHistory::push()`, `undo()`, and `redo()` do **not** block project signals. It's the caller's responsibility (`MainWindow::recalcDependents()`) to block signals during recalculation via `QSignalBlocker`.
- **Guard against reentry:** `MainWindow::m_recalcInProgress` prevents nested recalculation calls from signal cascades.

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
| `src/ui/MainWindow.h` | `m_cmdHistory` member, `onUndo()`, `onRedo()`, `updateUndoRedoActions()` |
| `src/ui/MainWindow.cpp` | `onUndo()` at line 822, `onRedo()` at line 845, `updateUndoRedoActions()` at line 868 |