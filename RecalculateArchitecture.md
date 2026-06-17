# ExpressDesigner — Recalculation Architecture
## Generated: June 2026

---

## 1. Overview

The recalculation system ensures that when a source object (data object or intermediate result) is modified — rotated, translated, edited at control-point level, or created by an operation — all downstream results are automatically recomputed. This is essential because the application models an **optical design pipeline** where operations form a dependency chain (e.g., `S1 → PropagateWF → WF1Propg → CalcOval → S2`).

The system uses a **DependencyGraph** to model object-to-operation-to-result relationships and a **transitive iterative algorithm** in `MainWindow::recalcDependents()` to cascade changes through the entire graph.

---

## 2. Core Classes

### 2.1 `DependencyGraph`
**File:** `src/core/DependencyGraph.h` / `src/core/DependencyGraph.cpp`

```cpp
class DependencyGraph : public QObject {
    // Object → Operations that use it as input
    QMap<QUuid, QVector<CustomOperation*>> m_objToOps;

    // Operation → Result object it produces
    QMap<CustomOperation*, CustomObject*> m_opToResult;

    // Result UUID → Operation that produced it
    QMap<QUuid, CustomOperation*> m_resultToOp;

    // UUID → Object lookup
    QMap<QUuid, CustomObject*> m_uuidToObj;
};
```

**Data flow representation:**

```
  m_objToOps                    m_opToResult               m_resultToOp
┌──────────────┐            ┌──────────────────┐        ┌──────────────────┐
│ uuid(S1)     │──→ [Op1]──→│  Op1 → WF1Propg  │        │ uuid(WF1Propg)   │──→ Op1
│ uuid(S2)     │──→ [Op2]   │  Op2 → Oval2     │        │ uuid(Oval2)      │──→ Op2
│ uuid(WF1Propg)──→ [Op2]  └──────────────────┘        └──────────────────┘
└──────────────┘
```

This shows that:
- S1 is used by Op1 (PropagateWF)
- WF1Propg is used by Op2 (CalcOval) — this is the transitive link
- Op1 produces WF1Propg
- Op2 produces Oval2

### 2.2 `DependencyGraph::rebuildFromProject()`

```cpp
void DependencyGraph::rebuildFromProject(Project* project) {
    clear();
    // 1. Register all objects (data + result) in m_uuidToObj
    for (auto* obj : project->dataObjects()) registerObject(obj);
    for (auto* obj : project->resultObjects()) registerObject(obj);

    // 2. Scan all operations
    const auto& ops = project->operations();
    for (auto* op : ops) {
        // 2a. For each parameter that references an object, register dependency
        for (int i = 0; i < op->paramCount(); ++i) {
            if (op->isParamObject(i)) {
                CustomObject* paramObj = project->findObject(op->paramName(i));
                if (paramObj) addObjectDependency(paramObj, op);  // obj → op
            }
        }
        // 2b. Register operation → result relationship
        CustomObject* resultObj = project->findObject(op->resultName());
        if (resultObj) addOperationResult(op, resultObj);  // op → result, result → op
    }
}
```

This is called **before and during** recalculation. It's a full scan — O(operations × parameters) — but operations and parameters are small in practice (typically < 50 operations with 1-3 object parameters each).

### 2.3 `DependencyGraph::transitiveDependents()`

```cpp
QSet<CustomObject*> DependencyGraph::transitiveDependents(CustomObject* obj) const {
    QSet<QUuid> visited;
    collectTransitive(obj->uuid(), visited);
    visited.remove(obj->uuid());  // exclude self
    // Map UUIDs back to object pointers
    QSet<CustomObject*> result;
    for (const QUuid& uid : visited) {
        CustomObject* depObj = m_uuidToObj.value(uid, nullptr);
        if (depObj) result.insert(depObj);
    }
    return result;
}
```

The helper `collectTransitive()` performs a **DFS** through operations and their results:

```
collectTransitive(startUuid):
    mark visited
    for each operation that uses this object:
        find the result of that operation
        recursively collectTransitive on the result
```

**Example:** Starting from S1:
1. S1 → Op1 (PropagateWF) → WF1Propg
2. WF1Propg → Op2 (CalcOval) → S2
3. S2 has no dependent operations → recursion ends

Result: `{WF1Propg, S2}` — both need recalculation when S1 changes.

### 2.4 `DependencyGraph::transitiveAncestors()`

Performs a **BFS upward** to find all source objects that ultimately affect a given result:

```cpp
QSet<CustomObject*> DependencyGraph::transitiveAncestors(CustomObject* obj) const {
    QSet<QUuid> visited;
    QList<QUuid> toVisit;
    toVisit.append(obj->uuid());
    while (!toVisit.isEmpty()) {
        QUuid current = toVisit.takeFirst();
        if (visited.contains(current)) continue;
        visited.insert(current);
        CustomOperation* parentOp = m_resultToOp.value(current, nullptr);
        if (!parentOp) continue;
        // Find all objects that use parentOp as dependency
        for (auto it = m_objToOps.begin(); it != m_objToOps.end(); ++it)
            if (it.value().contains(parentOp))
                toVisit.append(it.key());
    }
    visited.remove(obj->uuid());
    // Map back to objects
}
```

This is used by the DependencyGraphDialog to show the user which source objects a result depends on.

---

## 3. The Recalculation Algorithm: `MainWindow::recalcDependents()`

**File:** `src/ui/MainWindow.cpp`, line 886

```cpp
void MainWindow::recalcDependents(CustomObject* modifiedObj) {
    if (!m_currentProject || !m_depGraph) return;
    if (m_recalcInProgress) return;   // guard against reentry
    m_recalcInProgress = true;

    m_depGraph->rebuildFromProject(m_currentProject);

    if (!modifiedObj) { m_recalcInProgress = false; return; }

    // Transitive recalculation
    QSet<CustomOperation*> processed;   // ops already recalculated
    QSet<CustomObject*> modified;       // objects whose dependents need checking
    modified.insert(modifiedObj);

    const QSignalBlocker blocker(m_currentProject);  // suppress signals

    while (!modified.isEmpty()) {
        CustomObject* obj = *modified.begin();
        modified.erase(modified.begin());

        // Rebuild graph to see newly created results
        m_depGraph->rebuildFromProject(m_currentProject);

        QVector<CustomOperation*> ops = m_depGraph->operationsUsingObject(obj);
        for (auto* op : ops) {
            if (!op || processed.contains(op)) continue;
            processed.insert(op);

            // Remove old result by name
            QString resName = op->resultName();
            CustomObject* oldResult = m_currentProject->findObject(resName);
            if (oldResult) {
                m_currentProject->removeResultObject(oldResult);
                m_currentProject->removeDataObject(oldResult);
            }

            // Execute — creates new result
            op->execute(m_currentProject);

            // New result now needs its dependents checked too
            CustomObject* newResult = m_currentProject->findObject(resName);
            if (newResult)
                modified.insert(newResult);
        }
    }

    m_depGraph->rebuildFromProject(m_currentProject);
    m_recalcInProgress = false;
}
```

### 3.1 Algorithm Walkthrough (Pseudocode)

```
recalcDependents(modifiedObj):
    1. GUARD: if recalc already in progress, exit immediately
    2. SET flag m_recalcInProgress = true
    3. REBUILD dependency graph from current project state
    4. INIT: processedOps = {}           // ops already recomputed
    5. INIT: modifiedSet = {modifiedObj} // objects to check
    6. BLOCK all project signals (QSignalBlocker)

    7. WHILE modifiedSet is not empty:
        a. PICK and remove one object from modifiedSet
        b. REBUILD dependency graph (to see newly created results from previous iteration)
        c. FIND all operations that use this object as input
        d. FOR each operation not yet processed:
            i.   MARK operation as processed
            ii.  REMOVE old result by name (both from resultObjects and dataObjects)
            iii. EXECUTE the operation → creates new result object
            iv.  FIND the new result by name
            v.   ADD new result to modifiedSet (it may have its own dependents)

    8. FINAL rebuild of dependency graph
    9. CLEAR flag m_recalcInProgress = false
```

### 3.2 Key Design Decisions

**Why rebuild the graph inside the loop (step 7b)?**
The previous iteration may have created new result objects. These results weren't in the graph at the start of the loop. Rebuilding ensures the next iteration sees the current state, including newly created objects and their relationships to downstream operations.

**Why remove old result by name before executing (step 7d-ii)?**
This avoids auto-rename collisions. `Project::addResultObject()` auto-renames if a result with the same name exists. By removing the old result first, the new result gets the original name (no `_2` suffix).

**Why `removeDataObject` in addition to `removeResultObject` (step 7d-ii)?**
Results can exist in either list depending on project state. Both are checked and removed for safety.

**Why `QSignalBlocker` (step 6)?**
Project emits signals on every add/remove (`dataObjectAdded`, `resultObjectRemoved`, etc.). Without blocking, these signals would trigger UI updates (tree model rebuild, chart refresh) during the intermediate states of recalculation. Blocking ensures the UI only updates once, at the end, when `refreshChart()` is called.

**Why `m_recalcInProgress` guard (step 1)?**
Prevents reentrant calls. Without this, a signal emitted during recalculation could trigger another `recalcDependents()` call, leading to infinite recursion or inconsistent state.

**Why use a `QSet<CustomObject*>` for the work queue instead of a simple list?**
A set automatically deduplicates. If the same result object is reachable through multiple paths (unlikely but possible), it won't be processed twice.

---

## 4. Full Transitive Recalculation Example

### Setup
```
Project state:
  Data objects:   [S1, S2]
  Result objects: [WF1Propg, OvalS2]
  Operations:     [Op1(PropagateWF: S1→WF1Propg), Op2(CalcOval: WF1Propg→OvalS2)]

Dependency Graph:
  S1       → [Op1]
  WF1Propg → [Op2]
  Op1 → WF1Propg
  Op2 → OvalS2
```

### User rotates S1

```
MainWindow::onRotateObject():
  → RotateObjectCommand applied to S1
  → recalcDependents(S1)
```

### `recalcDependents(S1)` execution trace

```
Iteration 0 (initial):
  modifiedSet = {S1}
  processed   = {}

Iteration 1:
  obj = S1 (extracted from modifiedSet)
  modifiedSet = {}
  rebuild graph
  operationsUsingObject(S1) = [Op1]
  
  Process Op1:
    processed = {Op1}
    remove old result WF1Propg
    execute Op1 → creates new WF1Propg with S1's new geometry
    find newResult = WF1Propg
    modifiedSet = {WF1Propg}

Iteration 2:
  obj = WF1Propg (extracted from modifiedSet)
  modifiedSet = {}
  rebuild graph
  operationsUsingObject(WF1Propg) = [Op2]
  
  Process Op2:
    processed = {Op1, Op2}
    remove old result OvalS2
    execute Op2 → creates new OvalS2 with WF1Propg's new geometry
    find newResult = OvalS2
    modifiedSet = {OvalS2}

Iteration 3:
  obj = OvalS2 (extracted from modifiedSet)
  modifiedSet = {}
  rebuild graph
  operationsUsingObject(OvalS2) = []  // no downstream operations

  modifiedSet is empty → loop ends

Final:
  rebuild graph
  m_recalcInProgress = false
  refreshChart() → redraw all objects with updated geometry
```

### Result
- S1: rotated (geometry changed by RotateObjectCommand)
- WF1Propg: recalculated (new wavefront propagation from new S1)
- OvalS2: recalculated (new oval coupling from new WF1Propg)

---

## 5. Triggers — When Recalculation Happens

| User Action | Command Pushed? | Recalc Called? | Recalc Source |
|---|---|---|---|
| Insert object (via dialog) | Yes (`AddObjectCommand`) | No | N/A — no dependents yet |
| Delete object | Yes (`DeleteObjectCommand`) | After removing dependents (manual check in MainWindow) | `MainWindow::onDeleteObject()` |
| Rotate object | Yes (`RotateObjectCommand`) | Yes | `MainWindow::onRotateObject()` → `recalcDependents(obj)` |
| Translate object | Yes (`TranslateObjectCommand`) | Yes | `MainWindow::onTranslateObject()` → `recalcDependents(obj)` |
| Edit control points | Yes (`ModifyControlPointsCommand`) | Yes | `MainWindow::onEditPoints()` → `recalcDependents(obj)` |
| Execute CalcOval | Yes (`ExecuteOperationCommand`) | Yes | `MainWindow::onCalcOval()` → `recalcDependents(newResult)` |
| Execute PropagateWF | Yes (`ExecuteOperationCommand`) | Yes | `MainWindow::onPropagateWF()` → `recalcDependents(newResult)` |
| Undo (Ctrl+Z) | `undo()` called on CommandHistory | Yes, via `modifiedObjectName()` | `MainWindow::onUndo()` → `recalcDependents(modifiedObj)` |
| Redo (Ctrl+Y) | `redo()` called on CommandHistory | Yes, via `modifiedObjectName()` | `MainWindow::onRedo()` → `recalcDependents(modifiedObj)` |

### Delete Object Special Case
Deleting an object that is used by operations requires special handling — the dependent results must also be removed:

```cpp
// In MainWindow::onDeleteObject()
QSet<CustomObject*> deps = m_depGraph->transitiveDependents(obj);
// Warn user about dependents
// Remove dependent results
for (auto* dep : deps) {
    if (dep->isResult()) project->removeResultObject(dep);
    else project->removeDataObject(dep);
}
// Then remove the object itself
```

After deletion, `recalcDependents()` is **not** called — there's nothing to recalculate since the dependents were removed. But `m_depGraph->rebuildFromProject()` is still called to update the graph.

---

## 6. Integration with `CustomOperation`

**File:** `src/core/CustomOperation.h`

Operations are the "edges" in the dependency graph. Each operation:
- Has named parameters via `paramNames()` / `paramName(index)` — these are object names
- Knows which parameters reference objects via `isParamObject(index)`
- Has a `resultName()` — the name of the object it produces
- Has an `execute(Project*)` method that performs the computation and adds the result to the project

```cpp
class CustomOperation : public BaseObject {
    virtual bool execute(Project* project);
    virtual bool isParamObject(int index) const;
    virtual QString resultName() const = 0;
    virtual QString paramPrefixOnTree(int index) const;
};
```

Concrete operations like `CarthesianOvalOperation` and `PropagateWFOperation` implement:
- `execute()`: reads parameter objects from the project, performs optical computation (via SISL geometry library), creates a new `CurveObject` with computed control points, adds it to the project via `project->addResultObject()`.
- `isParamObject()`: returns true for parameters that are object references (vs numeric values).
- `resultName()`: returns the fixed result name (e.g., "WF1Propg").

---

## 7. Signal Flow During Recalculation

```
recalcDependents(S1)
  │
  ├─ QSignalBlocker(m_currentProject)        ← blocks ALL project signals
  │
  ├─ op1->execute(project)
  │   ├─ project->removeResultObject(old)    ← signal BLOCKED
  │   ├─ project->removeDataObject(old)      ← signal BLOCKED
  │   ├─ project->addResultObject(new)       ← signal BLOCKED
  │   └─ project->addOperation(op1)          ← signal BLOCKED
  │
  ├─ op2->execute(project)
  │   └─ (same pattern)
  │
  └─ QSignalBlocker destroyed                ← signals UNBLOCKED (no emission for past events)
  
  → refreshChart()                           ← explicit UI update
```

**Important:** `QSignalBlocker` only suppresses signals while in scope. When the blocker is destroyed at the end of `recalcDependents()`, **no queued signals are re-emitted** — they are simply lost. This is intentional: the intermediate add/remove events are irrelevant to the UI; only the final state matters.

After recalculation completes, `MainWindow` explicitly calls:
- `refreshChart()` — redraws all curves
- `updateStatusBar()` — updates object count
- `setModified(true)` — marks project as unsaved

And the `m_treeModel` (ObjectTreeModel) automatically reflects the project state because it queries `project->dataObjects()` and `project->resultObjects()` directly.

---

## 8. Thread Safety & Reentrancy

| Mechanism | Purpose |
|---|---|
| `m_recalcInProgress` (bool) | Prevents reentrant calls to `recalcDependents()`. Set at entry, cleared at exit. |
| `QSignalBlocker` | Suppresses all project signals during recalculation to prevent UI updates and signal cascades. |
| No threading | All recalculation runs on the Qt main thread. `Project` and `DependencyGraph` are not thread-safe and do not need to be. |
| `QSet<CustomOperation*> processed` | Prevents an operation from being executed twice within a single recalc cascade (safety net). |

---

## 9. Edge Cases & Design Decisions

1. **Operation execution failure:** If `op->execute(project)` returns false, the code does not currently handle the error explicitly — the old result was already removed. The new result won't be found by `findObject()` and won't be added to `modifiedSet`. The dependency chain breaks at that point, but downstream operations that depend on this result will have their input removed. A future improvement could add error recovery (re-insert the old result from a snapshot).

2. **Name collision during recalculation:** By removing the old result by name before executing, the new result gets the exact original name. `Project::addResultObject()` only auto-renames when a name collision exists within the object lists.

3. **Empty modifiedSet:** If `modifiedObj` is null (e.g., caller had no specific object), `recalcDependents()` exits immediately after rebuilding the graph. This is used in some code paths where only a graph rebuild is needed.

4. **Multiple operations producing the same result name:** This is not supported by design — each operation has a unique `resultName()`. If two operations produced results with the same name, the `removeResultObject` + `removeDataObject` calls would only remove one, and `addResultObject` would auto-rename the second.

5. **Recalculation during undo/redo of non-geometry commands:** For `AddObjectCommand`, `DeleteObjectCommand`, `ModifyObjectCommand`, `ModifyControlPointsCommand`, and `ExecuteOperationCommand`, `modifiedObjectName()` returns empty string. In `onUndo()`/`onRedo()`, `recalcDependents()` is only called if the name is non-empty. The dependency graph rebuild is still performed.

6. **Graph rebuild frequency:** The graph is rebuilt once at the beginning and once per iteration of the while loop. For a chain of N operations, this is N+2 rebuilds. Each rebuild is O(total_objects + total_operations × avg_params). For typical projects (< 100 objects, < 50 operations), this is negligible.

7. **Circular dependencies:** The current design does not explicitly prevent cycles (A → op1 → B → op2 → A). `collectTransitive()` tracks `visited` uuids to prevent infinite recursion, but a cycle would cause `recalcDependents()` to loop indefinitely because each iteration creates a new result that gets added to `modifiedSet`. In practice, the optical design domain does not create cycles (a wavefront cannot be the source for its own propagation).

---

## 10. File Index

| File | Purpose |
|---|---|
| `src/core/DependencyGraph.h` | Class declaration, data structures (`m_objToOps`, `m_opToResult`, `m_resultToOp`, `m_uuidToObj`) |
| `src/core/DependencyGraph.cpp` | `rebuildFromProject()`, `transitiveDependents()`, `transitiveAncestors()`, `collectTransitive()` |
| `src/core/CustomOperation.h` | Operation base class with `execute()`, `paramNames()`, `resultName()`, `isParamObject()` |
| `src/core/PropagateWFOperation.cpp` | PropagateWF operation — reads source object, propagates wavefront, creates result CurveObject |
| `src/core/CarthesianOvalOperation.cpp` / similar | CalcOval operation — reads two WFs, computes oval coupling, creates result |
| `src/core/Project.h` | `dataObjects()`, `resultObjects()`, `operations()`, `findObject()`, `addResultObject()`, `removeResultObject()` |
| `src/ui/MainWindow.h` | `m_depGraph`, `m_recalcInProgress`, `recalcDependents()` declaration |
| `src/ui/MainWindow.cpp` | `recalcDependents()` implementation (line 886), all call sites (rotate, translate, edit points, calcOval, propagateWF, undo, redo, delete) |
| `src/ui/widgets/PropertiesWidget.cpp` | `onSaveCalcOval()` and `onSavePropagate()` emit signals that trigger recalculation |