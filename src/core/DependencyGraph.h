#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include <QSet>
#include <QUuid>

namespace ExpressDesigner {

class CustomObject;
class CustomOperation;
class Project;

/**
 * @brief Directed graph tracking parent-child relationships between objects and operations.
 *
 * Edges:
 *   Object    → Operation   (object is an input parameter of the operation)
 *   Operation → Object      (object is the result of the operation)
 *
 * This enables:
 *   - Cascade recalculation when a source object is modified
 *   - Warning when deleting an object that has dependent results
 */
class DependencyGraph : public QObject {
    Q_OBJECT

public:
    explicit DependencyGraph(QObject* parent = nullptr);
    ~DependencyGraph() override;

    // --- Registration ---

    /** Register that @p obj is an input parameter for @p op. */
    void addObjectDependency(CustomObject* obj, CustomOperation* op);

    /** Register that @p op produced @p resultObj. */
    void addOperationResult(CustomOperation* op, CustomObject* resultObj);

    /** Remove all edges involving @p obj (when object is deleted). */
    void removeObject(CustomObject* obj);

    /** Remove all edges involving @p op (when operation is deleted). */
    void removeOperation(CustomOperation* op);

    // --- Queries ---

    /** All operations that use @p obj as a parameter. */
    QVector<CustomOperation*> operationsUsingObject(CustomObject* obj) const;

    /** The operation that produced @p resultObj (nullptr if none). */
    CustomOperation* parentOperationOf(CustomObject* resultObj) const;

    /** The result object produced by @p op (nullptr if none). */
    CustomObject* resultOfOperation(CustomOperation* op) const;

    /** All result objects that (transitively) depend on @p obj.
     *  Walks the graph: obj → op → result → nextOp → ... */
    QSet<CustomObject*> transitiveDependents(CustomObject* obj) const;

    /** All objects that @p obj depends on (its input ancestors). */
    QSet<CustomObject*> transitiveAncestors(CustomObject* obj) const;

    /** Clear all edges. */
    void clear();

    /** Rebuild the graph from a project's operations. */
    void rebuildFromProject(Project* project);

    /** Look up an object by UUID (requires graph was built from a project). */
    CustomObject* objectByUuid(const QUuid& uuid) const;

private:
    // Object UUID → Operations that use it
    QMap<QUuid, QVector<CustomOperation*>> m_objToOps;

    // Operation → Result object
    QMap<CustomOperation*, CustomObject*> m_opToResult;

    // Result object → Operation that produced it
    QMap<QUuid, CustomOperation*> m_resultToOp;

    // UUID → Object lookup (populated during rebuild)
    QMap<QUuid, CustomObject*> m_uuidToObj;

    void collectTransitive(const QUuid& startUuid, QSet<QUuid>& visited) const;
    void registerObject(CustomObject* obj);
};

} // namespace ExpressDesigner