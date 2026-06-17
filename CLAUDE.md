# CLAUDE.md - ExpressDesignerQt Coding Constraints

## Code Style Constraints

### No Nested IF Statements
- **Forbidden**: `if (a) { if (b) { ... } }` or `if (a && b) { if (c) { ... } }`
- **Use early returns/continues**: Exit or skip at the first violation, then continue with main logic.
- **Use guard clauses**: `if (!valid) return;` at function start to flatten logic.

### Prefer Smart Pointers
- **Use `std::unique_ptr`** for exclusive ownership (e.g., objects owned by Project).
- **Use `std::shared_ptr`** for shared ownership (e.g., objects referenced by both DependencyGraph and Project).
- **Avoid raw `new`/`delete`** except where Qt parent-child ownership already manages lifetime.
- **Qt objects with parent**: Raw pointers are acceptable because Qt's parent-child system handles deletion.

### Example - Before (nested if, raw pointers):
```cpp
void onDeleteObject() {
    if (idx.isValid() && m_currentProject) {
        CustomObject* obj = m_treeModel->objectAt(idx);
        if (obj) {
            if (m_depGraph) {
                QSet<CustomObject*> deps = m_depGraph->transitiveDependents(obj);
                if (!deps.isEmpty()) {
                    // warn user...
                }
            }
            // delete...
        }
    }
}
```

### Example - After (guard clauses, smart pointers):
```cpp
void onDeleteObject() {
    if (!idx.isValid() || !m_currentProject) return;
    CustomObject* obj = m_treeModel->objectAt(idx);
    if (!obj) return;

    if (m_depGraph) {
        QSet<CustomObject*> deps = m_depGraph->transitiveDependents(obj);
        if (!deps.isEmpty()) {
            // warn user...
            if (userCancelled) return;
        }
    }

    auto cmd = std::make_unique<DeleteObjectCommand>(obj, isResult);
    m_cmdHistory->push(cmd.get(), m_currentProject);
    cmd.release(); // cmdHistory takes ownership
}
```

## Project Architecture
See `ARCHITECTURE.md` for the full system design.