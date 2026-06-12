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
            QModelIndex parent = createIndex(0, 0, 1);
            beginInsertRows(parent, index, index);
            endInsertRows();
        });
        connect(m_project, &Project::dataObjectRemoved, this, [this](CustomObject*, int index) {
            QModelIndex parent = createIndex(0, 0, 1);
            beginRemoveRows(parent, index, index);
            endRemoveRows();
        });
        connect(m_project, &Project::resultObjectAdded, this, [this](CustomObject*, int index) {
            QModelIndex parent = createIndex(1, 0, 2);
            beginInsertRows(parent, index, index);
            endInsertRows();
        });
        connect(m_project, &Project::resultObjectRemoved, this, [this](CustomObject*, int index) {
            QModelIndex parent = createIndex(1, 0, 2);
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
    if (index.internalId() == 3) {
        int row = index.row();
        const auto& dataObjs = m_project->dataObjects();
        if (row < dataObjs.size()) return dataObjs[row];
        row -= dataObjs.size();
        const auto& resultObjs = m_project->resultObjects();
        if (row < resultObjs.size()) return resultObjs[row];
    }
    return nullptr;
}

QModelIndex ObjectTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!m_project) return QModelIndex();
    if (!parent.isValid()) {
        if (row == 0) return createIndex(row, column, 1); // Data folder
        if (row == 1) return createIndex(row, column, 2); // Result folder
        return QModelIndex();
    }
    if (parent.internalId() == 1 && row < m_project->dataObjectCount())
        return createIndex(row, column, 3);
    if (parent.internalId() == 2 && row < m_project->resultObjectCount())
        return createIndex(row, column, 3);
    return QModelIndex();
}

QModelIndex ObjectTreeModel::parent(const QModelIndex& child) const
{
    if (!child.isValid()) return QModelIndex();
    quintptr id = child.internalId();
    if (id == 1 || id == 2) return QModelIndex();
    if (id == 3) {
        int dataCount = m_project ? m_project->dataObjectCount() : 0;
        if (child.row() < dataCount) return createIndex(0, 0, 1);
        return createIndex(1, 0, 2);
    }
    return QModelIndex();
}

int ObjectTreeModel::rowCount(const QModelIndex& parent) const
{
    if (!m_project) return 0;
    if (!parent.isValid()) return 2;
    if (parent.internalId() == 1) return m_project->dataObjectCount();
    if (parent.internalId() == 2) return m_project->resultObjectCount();
    return 0;
}

int ObjectTreeModel::columnCount(const QModelIndex&) const { return 1; }

QVariant ObjectTreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return QVariant();
    if (role == Qt::DisplayRole) {
        quintptr id = index.internalId();
        if (id == 1) return QStringLiteral("Data Objects");
        if (id == 2) return QStringLiteral("Result Objects");
        if (id == 3) {
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
    if (idx >= 0) return createIndex(idx, 0, 3);
    const auto& resultObjs = m_project->resultObjects();
    idx = resultObjs.indexOf(obj);
    if (idx >= 0) return createIndex(idx, 0, 3);
    return QModelIndex();
}

} // namespace ExpressDesigner