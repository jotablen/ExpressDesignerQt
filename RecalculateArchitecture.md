# ExpressDesigner — Recalculation Architecture (Simplified)
## Generated: June 2026

---

## 1. Overview

The recalculation system ensures that when any object or operation parameter changes, **all results are recomputed from scratch** by re-executing every operation in creation order. This replaces the previous "surgical" approach that required a `DependencyGraph` inside the recalculation loop.

The key insight: **operations are already stored in `Project::m_operations` in dependency order** (the user creates them sequentially, each one depending on previously created results). Re-executing them in that order naturally propagates changes through the chain.

---

## 2. The Algorithm: `recalculateAll()`

```cpp
void MainWindow::recalculateAll()
{
    if (!m_currentProject) return;

    // 1. Block all project signals during recalculation
    const QSignalBlocker blocker(m_currentProject);

    // 2. Remove all current result objects
    //    Iterate backwards to avoid index invalidation
    for (int i = m_currentProject->resultObjectCount() - 1; i >= 0; --i)
        m_currentProject->removeResultObject(i);

    // 3. Re-execute every operation in insertion order
    const auto& ops = m_currentProject->operations();
    for (auto* op : ops) {
        if (!op) continue;
        bool ok = op->execute(m_currentProject);
        // If ok == false: operation could not execute
        //   (missing parameters, invalid indices, etc.)
        //   No result is created. Downstream operations that depend
        //   on this result's name will also fail → chain breaks cleanly.
        // If ok == true: result was added to project.
        //   It may have 0 control points (e.g., no ray-surface hits).
        //   Downstream operations receive it as-is.
    }

    // 4. Rebuild dependency graph ONCE for UI queries (dialog, delete warnings)
    if (m_depGraph)
        m_depGraph->rebuildFromProject(m_currentProject);

    // 5. Update UI
    refreshChart();
    updateStatusBar();
    setModified(true);
}
```

### 2.1 Why this works

Operations resolve their inputs **by name** at execution time:

```cpp
// Inside PropagateWFOperation::execute():
CustomObject* wf = project->findObject(m_paramNames.value(PARAM_WF));
CustomObject* surface = project->findObject(m_paramNames.value(PARAM_SURFACE));
```

When operations execute in order:

1. Op1 executes → finds its inputs (data objects S1, S2) → creates result "WF1Propg"
2. Op2 executes → finds its input "WF1Propg" (just created by Op1) → creates result "Oval"
3. Op3 executes → finds its inputs... etc.

The insertion order in `m_operations` already mirrors the dependency chain because the user creates operations sequentially.

### 2.2 What disappears compared to the old design

| Old (surgical) | New (total) |
|---|---|
| `recalcDependents(CustomObject* modifiedObj)` — 50 lines | `recalculateAll()` — 15 lines |
| `m_recalcInProgress` guard | Not needed — signals are blocked |
| `QSet<CustomOperation*> processed` | Not needed |
| `QSet<CustomObject*> modified` work queue | Not needed |
| Graph rebuild inside the `while` loop | 1 rebuild at the end |
| `Command::modifiedObjectName()` as recalc bridge | Not needed |
| `lastUndoneModifiedObjectName()` / `lastRedoneModifiedObjectName()` | Not needed |

---

## 3. Failure Handling

The user asked about cases where "after a property change, results can no longer be calculated." Here's how each case behaves:

### 3.1 Missing input object

**Scenario:** Object "S1" is deleted. An operation references "S1" as a parameter.

```cpp
// Inside operation::execute():
CustomObject* wf = project->findObject("S1");  // → nullptr
if (!wf || !surface) {
    m_errorCode = -2;
    return false;  // execute() returns false
}
```

**Result:** `execute()` returns `false`. No result object is added to the project. Downstream operations that reference this result by name will also get `nullptr` from `findObject()` and return `false`. The entire chain downstream of the missing object fails cleanly.

### 3.2 Invalid refractive index

**Scenario:** User sets `refractiveIndex` to 0 or a negative value on a wavefront.

```cpp
double n1 = wf->refractiveIndex();  // → 0.0
// Snell's law: division by n1 would be problematic
// The operation should validate and set m_errorCode
```

**Current behavior:** The code uses `n1` in `OPL = n1 * hitDist` and later in Snell's law. If `n1` is 0, the optical path length is 0, and Snell's law division could produce infinity or NaN. The operation's `execute()` should validate `n1 > 0` and return `false` if invalid.

**Proposed improvement:** Add validation at the top of each `execute()`:

```cpp
if (n1 <= 0.0 || n2 <= 0.0) {
    m_errorCode = -3;  // Invalid refractive index
    return false;
}
```

### 3.3 Geometry change makes ray-surface intersection impossible

**Scenario:** User rotates/translates S2 so far that rays from the WF no longer hit the surface.

```cpp
// In PropagateWFOperation::execute():
for (int i = 0; i < numPoints; ++i) {
    QPointF surfHit = rayCurveIntersection(srcPt, norm, surfPts, &hitSeg, &hitDist);
    if (hitSeg < 0) {
        continue;  // No hit — skip this point
    }
    // ... compute propagation
}
```

**Result:** `propagatedPts` ends up empty (or partially empty). A `CurveObject` with 0 control points is created and added to the project. Downstream operations (e.g., CalcOval) receive a curve with 0 points and also produce an empty result. The chart will simply not render anything for those curves — graceful degradation.

### 3.4 Total Internal Reflection (TIR)

**Scenario:** The incidence angle at the surface exceeds the critical angle.

```cpp
bool tir = false;
QPointF deflectedDir = Optics::getDeflectedVector(norm, surfNormal, n1, n2, tir);
if (!tir)
    remaining /= n2;  // transmitted
// If TIR, the point is still added but with remaining/n2? No:
// Actually, the code continues regardless — TIR is detected but handled
```

**Current behavior:** The TIR flag is set but the deflected vector is still computed. The result may be physically incorrect for TIR cases. This is an optics domain concern, not a recalculation architecture concern.

### 3.5 Summary of failure modes

| Failure | execute() returns | Result object created? | Downstream impact |
|---|---|---|---|
| Missing input object | `false` | No | findObject() → nullptr → downstream also fails |
| Invalid refractiveIndex | `false` (after validation) | No | Same as above |
| No ray-surface intersection | `true` | Yes, empty CurveObject | Downstream processes empty input → empty output |
| TIR on all rays | `true` | Yes, CurveObject with TIR-deflected points | Downstream uses these points |
| Operation calculation error | `false` via `m_errorCode` | No | Chain breaks |

**Key property:** The system degrades gracefully. If an operation fails, it doesn't corrupt the project — it simply produces no result, and the chain stops there. The chart refreshes with whatever valid results exist.

---

## 4. Integration with Undo/Redo

In the simplified design, undo and redo call `recalculateAll()` directly without needing `modifiedObjectName()`:

```
MainWindow::onUndo()
  → m_cmdHistory->undo(project)        // restores previous state
  → recalculateAll()                   // re-executes all operations
  → refreshChart()
  → updateStatusBar()
  → setModified(true)

MainWindow::onRedo()
  → m_cmdHistory->redo(project)        // re-applies state
  → recalculateAll()                   // re-executes all operations
  → refreshChart()
  → updateStatusBar()
  → setModified(true)
```

The `modifiedObjectName()` / `lastUndoneModifiedObjectName()` mechanism is no longer needed as a bridge between undo/redo and recalculation.

---

## 5. Triggers — All Use `recalculateAll()`

| User Action | Command Pushed? | Recalc Called? |
|---|---|---|
| Insert object | Yes (`AddObjectCommand`) | No — no operations yet |
| Delete object | Yes (`DeleteObjectCommand`) | Yes, after removing dependents |
| Rotate object | Yes (`RotateObjectCommand`) | Yes, `recalculateAll()` |
| Translate object | Yes (`TranslateObjectCommand`) | Yes, `recalculateAll()` |
| Edit control points | Yes (`ModifyControlPointsCommand`) | Yes, `recalculateAll()` |
| Execute CalcOval | Yes (`ExecuteOperationCommand`) | Yes, `recalculateAll()` |
| Execute PropagateWF | Yes (`ExecuteOperationCommand`) | Yes, `recalculateAll()` |
| Undo (Ctrl+Z) | `undo()` on CommandHistory | Yes, `recalculateAll()` |
| Redo (Ctrl+Y) | `redo()` on CommandHistory | Yes, `recalculateAll()` |

### Delete Object Special Case

When deleting an object that is used by operations, the dependent results must be removed first. `DependencyGraph::transitiveDependents()` is used for this (its only remaining runtime role):

```cpp
// In MainWindow::onDeleteObject()
QSet<CustomObject*> deps = m_depGraph->transitiveDependents(obj);
// Warn user: "These results will be lost: ..."
// Remove dependent results
for (auto* dep : deps)
    project->removeResultObject(dep);
// Remove the object itself
project->removeDataObject(obj);
// Rebuild graph, recalculateAll, refreshChart
```

---

## 6. `DependencyGraph` — Remaining Role

`DependencyGraph` is **no longer part of the recalculation engine**. Its only uses are:

| Use | Method |
|---|---|
| Dependency visualization dialog | `transitiveDependents()` + `transitiveAncestors()` |
| Delete warning (show users what will be lost) | `transitiveDependents()` |
| Graph rebuild after load | `rebuildFromProject()` (called once on project load) |

The graph is rebuilt **once** at the end of `recalculateAll()` to keep the UI dialogs in sync.

---

## 7. Signal Flow

```
recalculateAll()
  │
  ├─ QSignalBlocker(m_currentProject)        ← blocks ALL project signals
  │
  ├─ removeResultObject(0)                   ← signal BLOCKED
  ├─ removeResultObject(1)                   ← signal BLOCKED
  │   ...
  ├─ op1->execute(project)                   ← signal BLOCKED (emits operationExecuted)
  │   └─ project->addResultObject(result1)   ← signal BLOCKED
  ├─ op2->execute(project)                   ← signal BLOCKED
  │   └─ project->addResultObject(result2)   ← signal BLOCKED
  │   ...
  └─ QSignalBlocker destroyed                ← signals UNBLOCKED

  → m_depGraph->rebuildFromProject()         ← single graph rebuild
  → refreshChart()                           ← explicit UI redraw
  → updateStatusBar()
```

No queued signals are re-emitted after the blocker is destroyed — intermediate add/remove events are irrelevant to the UI.

---

## 8. Performance

For a project with N operations, each with M parameters:

| Metric | Value |
|---|---|
| Removals | O(R) where R = number of result objects |
| Re-executions | N |
| Each execute() | O(points × intersection_checks) |
| Graph rebuilds | 1 (at end) |
| Total complexity | O(N × M_points) |

For typical projects (N < 50, M_points < 200), this runs in milliseconds. The `QSignalBlocker` ensures no UI overhead during the process.

---

## 9. Edge Cases

1. **Empty project:** `m_operations` is empty → loop does nothing. `refreshChart()` redraws data objects only.

2. **Operation with no params:** `execute()` receives `nullptr` for inputs → returns `false`. Result not created.

3. **Multiple operations with same `resultName`:** The second `addResultObject()` auto-renames via `Project`'s deduplication logic (appends `_2`). Downstream ops looking for the original name won't find the renamed result → they fail. This is correct: two operations shouldn't share a result name.

4. **Operation execution failure mid-chain:** Operations after the failure will `findObject()` the expected result name and get `nullptr` → they also fail. The chain breaks cleanly without partial state.

5. **Reentrancy:** `QSignalBlocker` prevents signal-based reentry. If a direct recursive call somehow occurs, the second call removes results and re-executes from scratch — wasteful but not corrupting.

---

## 10. File Index

| File | Purpose |
|---|---|
| `src/core/Project.h` / `.cpp` | `m_operations` (ordered list), `dataObjects()`, `resultObjects()`, `addResultObject()`, `removeResultObject()` |
| `src/core/CustomOperation.h` | `execute(Project*)`, `paramName()`, `resultName()`, `isParamObject()` |
| `src/core/PropagateWFOperation.cpp` | Concrete `execute()` — reads inputs by name, computes propagation, adds result |
| `src/core/CarthesianOvalOperation.cpp` | Concrete `execute()` — reads two WFs, computes oval coupling, adds result |
| `src/core/DependencyGraph.h` / `.cpp` | `rebuildFromProject()`, `transitiveDependents()`, `transitiveAncestors()` — UI only |
| `src/ui/MainWindow.h` | `m_depGraph`, `recalculateAll()` declaration |
| `src/ui/MainWindow.cpp` | `recalculateAll()` implementation, all call sites |