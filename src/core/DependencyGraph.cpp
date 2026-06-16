#include "DependencyGraph.h"
#include "CustomObject.h"
#include "CustomOperation.h"
#include "Project.h"

namespace ExpressDesigner {

DependencyGraph::DependencyGraph(QObject* parent) : QObject(parent) {}
DependencyGraph::~DependencyGraph() = default;

void DependencyGraph::registerObject(CustomObject* obj)
{
    if (!obj) return;
    m_uuidToObj[obj->uuid()] = obj;
}

CustomObject* DependencyGraph::objectByUuid(const QUuid& uuid) const
{
    return m_uuidToObj.value(uuid, nullptr);
}

void DependencyGraph::addObjectDependency(CustomObject* obj, CustomOperation* op)
{
    if (!obj || !op) return;
    QVector<CustomOperation*>& ops = m_objToOps[obj->uuid()];
    if (!ops.contains(op))
        ops.append(op);
}

void DependencyGraph::addOperationResult(CustomOperation* op, CustomObject* resultObj)
{
    if (!op || !resultObj) return;
    m_opToResult[op] = resultObj;
    m_resultToOp[resultObj->uuid()] = op;
}

void DependencyGraph::removeObject(CustomObject* obj)
{
    if (!obj) return;
    QUuid uid = obj->uuid();

    m_objToOps.remove(uid);
    m_uuidToObj.remove(uid);

    if (m_resultToOp.contains(uid)) {
        CustomOperation* op = m_resultToOp.take(uid);
        m_opToResult.remove(op);
    }

    // Clean up any nullptrs
    for (auto it = m_objToOps.begin(); it != m_objToOps.end(); ) {
        it.value().removeAll(nullptr);
        if (it.value().isEmpty()) {
            it = m_objToOps.erase(it);
        } else {
            ++it;
        }
    }
}

void DependencyGraph::removeOperation(CustomOperation* op)
{
    if (!op) return;

    if (m_opToResult.contains(op)) {
        CustomObject* result = m_opToResult.take(op);
        if (result) m_resultToOp.remove(result->uuid());
    }

    for (auto& ops : m_objToOps) {
        ops.removeAll(op);
    }
}

QVector<CustomOperation*> DependencyGraph::operationsUsingObject(CustomObject* obj) const
{
    if (!obj) return {};
    return m_objToOps.value(obj->uuid());
}

CustomOperation* DependencyGraph::parentOperationOf(CustomObject* resultObj) const
{
    if (!resultObj) return nullptr;
    return m_resultToOp.value(resultObj->uuid(), nullptr);
}

CustomObject* DependencyGraph::resultOfOperation(CustomOperation* op) const
{
    if (!op) return nullptr;
    return m_opToResult.value(op, nullptr);
}

void DependencyGraph::collectTransitive(const QUuid& startUuid, QSet<QUuid>& visited) const
{
    if (visited.contains(startUuid)) return;
    visited.insert(startUuid);

    // Follow: object → ops that use it → their results
    const auto& ops = m_objToOps.value(startUuid);
    for (auto* op : ops) {
        if (!op) continue;
        CustomObject* result = m_opToResult.value(op, nullptr);
        if (result) {
            collectTransitive(result->uuid(), visited);
        }
    }
}

QSet<CustomObject*> DependencyGraph::transitiveDependents(CustomObject* obj) const
{
    QSet<CustomObject*> result;
    if (!obj) return result;

    QSet<QUuid> visited;
    collectTransitive(obj->uuid(), visited);
    visited.remove(obj->uuid());

    for (const QUuid& uid : visited) {
        CustomObject* depObj = m_uuidToObj.value(uid, nullptr);
        if (depObj) result.insert(depObj);
    }
    return result;
}

QSet<CustomObject*> DependencyGraph::transitiveAncestors(CustomObject* obj) const
{
    QSet<CustomObject*> result;
    if (!obj) return result;

    // Walk backwards: obj → parent op → input objs → their parent ops → ...
    QSet<QUuid> visited;
    QList<QUuid> toVisit;

    if (m_resultToOp.contains(obj->uuid())) {
        toVisit.append(obj->uuid());
    }

    while (!toVisit.isEmpty()) {
        QUuid current = toVisit.takeFirst();
        if (visited.contains(current)) continue;
        visited.insert(current);

        CustomOperation* parentOp = m_resultToOp.value(current, nullptr);
        if (!parentOp) continue;

        // Find all objects that feed into this operation
        for (auto it = m_objToOps.begin(); it != m_objToOps.end(); ++it) {
            if (it.value().contains(parentOp)) {
                toVisit.append(it.key());
            }
        }
    }

    visited.remove(obj->uuid());
    for (const QUuid& uid : visited) {
        CustomObject* anc = m_uuidToObj.value(uid, nullptr);
        if (anc) result.insert(anc);
    }
    return result;
}

void DependencyGraph::clear()
{
    m_objToOps.clear();
    m_opToResult.clear();
    m_resultToOp.clear();
    m_uuidToObj.clear();
}

void DependencyGraph::rebuildFromProject(Project* project)
{
    clear();
    if (!project) return;

    // Register all objects for UUID lookup
    for (auto* obj : project->dataObjects()) {
        registerObject(obj);
    }
    for (auto* obj : project->resultObjects()) {
        registerObject(obj);
    }

    const auto& ops = project->operations();
    for (auto* op : ops) {
        if (!op) continue;

        for (int i = 0; i < op->paramCount(); ++i) {
            if (op->isParamObject(i)) {
                QString paramName = op->paramName(i);
                CustomObject* paramObj = project->findObject(paramName);
                if (paramObj) {
                    addObjectDependency(paramObj, op);
                }
            }
        }

        QString resName = op->resultName();
        CustomObject* resultObj = project->findObject(resName);
        if (resultObj) {
            addOperationResult(op, resultObj);
        }
    }
}

} // namespace ExpressDesigner