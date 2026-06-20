#include "ObjectTreeModel.h"

namespace ExpressDesigner {

ObjectTreeModel::ObjectTreeModel(QObject* parent) : QAbstractItemModel(parent) {}

void ObjectTreeModel::setProject(Project* project)
{
    beginResetModel();

    if (m_project) {
        disconnect(m_project, nullptr, this, nullptr);
    }

    m_project = project;

    if (m_project) {
        connect(m_project, &Project::dataObjectAdded, this, [this](CustomObject*, int index) {
            QModelIndex parent = createIndex(0, 0, kDataFolderId);
            beginInsertRows(parent, index, index);
            endInsertRows();
        });
        connect(m_project, &Project::dataObjectRemoved, this, [this](CustomObject*, int index) {
            QModelIndex parent = createIndex(0, 0, kDataFolderId);
            beginRemoveRows(parent, index, index);
            endRemoveRows();
        });
        connect(m_project, &Project::resultObjectAdded, this, [this](CustomObject*, int index) {
            QModelIndex parent = createIndex(1, 0, kResultFolderId);
            beginInsertRows(parent, index, index);
            endInsertRows();
        });
        connect(m_project, &Project::resultObjectRemoved, this, [this](CustomObject*, int index) {
            QModelIndex parent = createIndex(1, 0, kResultFolderId);
            beginRemoveRows(parent, index, index);
            endRemoveRows();
        });
        connect(m_project, &Project::operationAdded, this, [this](CustomOperation*, int index) {
            QModelIndex parent = createIndex(2, 0, kOpsFolderId);
            beginInsertRows(parent, index, index);
            endInsertRows();
        });
        connect(m_project, &Project::operationRemoved, this, [this](CustomOperation*, int index) {
            QModelIndex parent = createIndex(2, 0, kOpsFolderId);
            beginRemoveRows(parent, index, index);
            endRemoveRows();
        });
        // Refresh header when project name changes
        connect(m_project, &BaseObject::nameChanged, this, [this](const QString&) {
            emit headerDataChanged(Qt::Horizontal, 0, 0);
        });
    }

    endResetModel();
}

CustomObject* ObjectTreeModel::objectAt(const QModelIndex& index) const
{
    if (!m_project || !index.isValid()) return nullptr;
    auto id = index.internalId();
    if (id == kDataChildId) {
        const auto& dataObjs = m_project->dataObjects();
        int row = index.row();
        if (row >= 0 && row < dataObjs.size()) return dataObjs[row];
    }
    if (id == kResultChildId) {
        const auto& resultObjs = m_project->resultObjects();
        int row = index.row();
        if (row >= 0 && row < resultObjs.size()) return resultObjs[row];
    }
    return nullptr;
}

CustomOperation* ObjectTreeModel::operationAt(const QModelIndex& index) const
{
    if (!m_project || !index.isValid()) return nullptr;
    if (index.internalId() == kOpsChildId) {
        const auto& ops = m_project->operations();
        int row = index.row();
        if (row >= 0 && row < ops.size()) return ops[row];
    }
    return nullptr;
}

QModelIndex ObjectTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!m_project) return QModelIndex();
    if (!parent.isValid()) {
        // Root level: 3 folders
        if (row >= 0 && row < 3) return createIndex(row, column, kDataFolderId + row);
        return QModelIndex();
    }
    quintptr pId = parent.internalId();
    if (pId == kDataFolderId && row < m_project->dataObjectCount())
        return createIndex(row, column, kDataChildId);
    if (pId == kResultFolderId && row < m_project->resultObjectCount())
        return createIndex(row, column, kResultChildId);
    if (pId == kOpsFolderId && row < m_project->operations().size())
        return createIndex(row, column, kOpsChildId);
    return QModelIndex();
}

QModelIndex ObjectTreeModel::parent(const QModelIndex& child) const
{
    if (!child.isValid()) return QModelIndex();
    quintptr id = child.internalId();
    if (isFolderId(id)) return QModelIndex();
    if (id == kDataChildId) return createIndex(0, 0, kDataFolderId);
    if (id == kResultChildId) return createIndex(1, 0, kResultFolderId);
    if (id == kOpsChildId) return createIndex(2, 0, kOpsFolderId);
    return QModelIndex();
}

int ObjectTreeModel::rowCount(const QModelIndex& parent) const
{
    if (!m_project) return 0;
    if (!parent.isValid()) return 3;  // Data, Result, Operations
    quintptr id = parent.internalId();
    if (id == kDataFolderId) return m_project->dataObjectCount();
    if (id == kResultFolderId) return m_project->resultObjectCount();
    if (id == kOpsFolderId) return m_project->operations().size();
    return 0;
}

int ObjectTreeModel::columnCount(const QModelIndex&) const { return 1; }

QVariant ObjectTreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return QVariant();
    if (role == Qt::DisplayRole) {
        quintptr id = index.internalId();
        if (id == kDataFolderId) return QStringLiteral("Data Objects");
        if (id == kResultFolderId) return QStringLiteral("Result Objects");
        if (id == kOpsFolderId) return QStringLiteral("Operations");
        if (id == kDataChildId || id == kResultChildId) {
            auto* obj = objectAt(index);
            return obj ? obj->name() : QVariant();
        }
        if (id == kOpsChildId) {
            auto* op = operationAt(index);
            return op ? op->name() : QVariant();
        }
    }
    return QVariant();
}

QVariant ObjectTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return m_project ? m_project->name() : QStringLiteral("Name");
    return QVariant();
}

QModelIndex ObjectTreeModel::createIndexByObject(CustomObject* obj) const
{
    if (!m_project || !obj) return QModelIndex();
    const auto& dataObjs = m_project->dataObjects();
    int idx = dataObjs.indexOf(obj);
    if (idx >= 0) return createIndex(idx, 0, kDataChildId);
    const auto& resultObjs = m_project->resultObjects();
    idx = resultObjs.indexOf(obj);
    if (idx >= 0) return createIndex(idx, 0, kResultChildId);
    return QModelIndex();
}

} // namespace ExpressDesigner