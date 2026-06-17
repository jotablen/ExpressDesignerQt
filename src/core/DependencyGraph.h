#pragma once
#include <QObject>
#include <QVector>
#include <QMap>
#include <QSet>
#include <QUuid>
#include <memory>

namespace ExpressDesigner {

class CustomObject;
class CustomOperation;
class Project;

class DependencyGraph : public QObject {
    Q_OBJECT
public:
    explicit DependencyGraph(QObject* parent = nullptr);
    ~DependencyGraph() override;

    void addObjectDependency(CustomObject* obj, CustomOperation* op);
    void addOperationResult(CustomOperation* op, CustomObject* resultObj);
    void removeObject(CustomObject* obj);
    void removeOperation(CustomOperation* op);

    QVector<CustomOperation*> operationsUsingObject(CustomObject* obj) const;
    CustomOperation* parentOperationOf(CustomObject* resultObj) const;
    CustomObject* resultOfOperation(CustomOperation* op) const;
    QSet<CustomObject*> transitiveDependents(CustomObject* obj) const;
    QSet<CustomObject*> transitiveAncestors(CustomObject* obj) const;

    void clear();
    void rebuildFromProject(Project* project);
    CustomObject* objectByUuid(const QUuid& uuid) const;

private:
    using OpList = QVector<CustomOperation*>;
    QMap<QUuid, OpList> m_objToOps;
    QMap<CustomOperation*, CustomObject*> m_opToResult;
    QMap<QUuid, CustomOperation*> m_resultToOp;
    QMap<QUuid, CustomObject*> m_uuidToObj;

    void collectTransitive(const QUuid& uuid, QSet<QUuid>& visited) const;
    void registerObject(CustomObject* obj);
};

} // namespace ExpressDesigner