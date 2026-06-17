# ExpressDesigner - Strategy for Undo/Redo and Dependency System
# Generated: June 2026

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                      MainWindow                              │
│  ┌──────────┐  ┌──────────────┐  ┌────────────────────┐    │
│  │HistoryMgr│  │DependencyGraph│  │CommandHistory       │    │
│  │ (log)    │  │(obj→op→result)│  │(undo/redo stack)   │    │
│  └──────────┘  └──────────────┘  └────────────────────┘    │
│                                                     │       │
│  ┌──────────────────────────────────────────────────┐      │
│  │              Project (model)                      │      │
│  │  ┌─────────────┐  ┌──────────────┐  ┌──────────┐ │      │
│  │  │ dataObjects  │  │ resultObjects │  │operations│ │      │
│  │  └─────────────┘  └──────────────┘  └──────────┘ │      │
│  └──────────────────────────────────────────────────┘      │
└─────────────────────────────────────────────────────────────┘
```

## Undoable Actions (all stored in CommandHistory stack)

| Action        | Command Class             | What it stores                              | modifiedObjectName()           |
|---------------|---------------------------|---------------------------------------------|-------------------------------|
| Insert object | AddObjectCommand          | CustomObject* pointer, wasResult flag       | "" (none)                     |
| Delete object | DeleteObjectCommand       | CustomObject* pointer, index, isResult flag | "" (none)                     |
| Rotate object | RotateObjectCommand       | obj*, oldPoints, pivot, degrees             | obj→name()                    |
| Translate obj | TranslateObjectCommand    | obj*, delta                                 | obj→name()                    |
| Propagate WF  | ExecuteOperationCommand   | CustomOperation* pointer                    | "" (none)                     |
| Calc Oval     | ExecuteOperationCommand   | CustomOperation* pointer                    | "" (none)                     |
| Modify prop.  | ModifyObjectCommand       | obj*, propertyName, oldValue, newValue      | "" (none)                     |
| Mod. ctrl pts | ModifyControlPointsCmd    | obj*, oldPoints, newPoints                  | "" (none)                     |

## Key Rules

1. **All mutations go through CommandHistory.push()**
   - push() calls cmd->execute() internally
   - On success, pushes to undo stack
   - Emits `stackChanged()` → `MainWindow::updateUndoRedoActions()` keeps menu captions in sync

2. **RecalcDependents is NOT a command**
   - It is a side-effect executed directly after rotate/translate/modify
   - It does NOT push anything to CommandHistory
   - It blocks Project signals during execution to avoid cascading

3. **Undo/Redo triggers recalcDependents via modifiedObjectName()**
   - `Command::modifiedObjectName()` returns the name of the object affected by the command
   - `RotateObjectCommand` and `TranslateObjectCommand` return the object name
   - After undo/redo, `CommandHistory::lastUndoneModifiedObjectName()` / `lastRedoneModifiedObjectName()` returns the name
   - `MainWindow` resolves the name via `Project::findObject()` and calls `recalcDependents()`
   - This ensures that undoing a Rotate/Translate on S1 also recalculates WF1Propg and Oval

4. **Transitive recalculation**
   - When obj A is modified/rotated/translated:
     a. Find all operations that use A as parameter
     b. Remove old results by name
     c. Re-execute each operation
     d. Add new results to "modified" set
     e. Repeat from (a) for each new result
   - This ensures:
     S1 → PropWV → WF1Propg → CalcOval → S2
     Rotating S1 recalculates BOTH WF1Propg AND S2

5. **No auto-rename during recalc**
   - Old result is removed by name BEFORE executing
   - New result gets the original name (no _2 suffix)

## Execution Flow: Rotate Object

```
onRotateObject()
  → RotateObjectCommand::execute()     ← records old pts, applies new
  → CommandHistory::push(cmd)          ← pushes to undo stack
  → recalcDependents(obj)
      → DependencyGraph::rebuildFromProject()
      → while (objects to check):
          → find operations using object
          → remove old result by name
          → op->execute()               ← PropagateWF::execute()
          → add new result to modified set
      → DependencyGraph::rebuildFromProject()
```

## Execution Flow: Propagate WF

```
onPropagateWF()
  → new PropagateWFOperation(name)
  → new ExecuteOperationCommand(op)
  → CommandHistory::push(execCmd)       ← cmd->execute() calls op->execute()
      → op->execute()
          → new CurveObject(resultName)   ← created by operation
          → project->addResultObject()    ← added to model
          → emit operationExecuted(true)
  → project->addOperation(op)           ← operation becomes a "recipe"
  → DependencyGraph::rebuildFromProject()
```

## Execution Flow: Undo

```
onUndo()
  → m_cmdHistory->undo(project)         ← restores previous state
  → DependencyGraph::rebuildFromProject()  ← re-scans operations
  → cmdHistory->lastUndoneModifiedObjectName()  ← get affected object name
  → project->findObject(objName)        ← resolve name safely (no raw pointer)
  → recalcDependents(modifiedObj)       ← cascade-recalculate dependents
  → refreshChart()
```

## Data Structures

### DependencyGraph
- m_objToOps:   QUuid → QVector<CustomOperation*>   (which ops use this object)
- m_opToResult: CustomOperation* → CustomObject*     (what result did op produce)
- m_resultToOp: QUuid → CustomOperation*             (which op produced this result)
- m_uuidToObj:  QUuid → CustomObject*                (UUID lookup)

### CommandHistory
- m_undoStack: std::vector<std::unique_ptr<Command>>   (up to 200)
- m_redoStack: std::vector<std::unique_ptr<Command>>   (cleared on new command)
- stackChanged() signal → connected to MainWindow::updateUndoRedoActions()
- lastUndoneModifiedObjectName() → QString (object name from redo stack top)
- lastRedoneModifiedObjectName() → QString (object name from undo stack top)

### Command base class
- execute(Project*) = 0
- undo(Project*) = 0
- description() → "Rotate MyObject", "Delete MyObject", etc.
- modifiedObjectName() → QString (returns object name for recalc; empty by default)
