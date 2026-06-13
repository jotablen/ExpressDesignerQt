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
        // Refresh display when any object changes
        connect(m_project, &QObject::objectNameChanged, this, [this]() {
            emit dataChanged(index(0, 0), index(rowCount() - 1, 0));
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

QModelIndex ObjectTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!m_project) return QModelIndex();
    if (!parent.isValid()) {
        if (row == 0) return createIndex(row, column, kDataFolderId);
        if (row == 1) return createIndex(row, column, kResultFolderId);
        return QModelIndex();
    }
    if (parent.internalId() == kDataFolderId && row < m_project->dataObjectCount())
        return createIndex(row, column, kDataChildId);
    if (parent.internalId() == kResultFolderId && row < m_project->resultObjectCount())
        return createIndex(row, column, kResultChildId);
    return QModelIndex();
}

QModelIndex ObjectTreeModel::parent(const QModelIndex& child) const
{
    if (!child.isValid()) return QModelIndex();
    quintptr id = child.internalId();
    if (id == kDataFolderId || id == kResultFolderId) return QModelIndex();
    if (id == kDataChildId) return createIndex(0, 0, kDataFolderId);
    if (id == kResultChildId) return createIndex(1, 0, kResultFolderId);
    return QModelIndex();
}

int ObjectTreeModel::rowCount(const QModelIndex& parent) const
{
    if (!m_project) return 0;
    if (!parent.isValid()) return 2;
    if (parent.internalId() == kDataFolderId) return m_project->dataObjectCount();
    if (parent.internalId() == kResultFolderId) return m_project->resultObjectCount();
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
        if (id == kDataChildId || id == kResultChildId) {
            auto* obj = objectAt(index);
            return obj ? obj->name() : QVariant();
        }
    }
    return QVariant();
}

QVariant ObjectTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return QStringLiteral("Name");
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