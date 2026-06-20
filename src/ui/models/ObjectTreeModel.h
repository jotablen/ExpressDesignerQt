#pragma once
#include <QAbstractItemModel>
#include <core/Project.h>
#include <core/CustomObject.h>
#include <core/CustomOperation.h>

namespace ExpressDesigner {

class ObjectTreeModel : public QAbstractItemModel {
    Q_OBJECT
public:
    explicit ObjectTreeModel(QObject* parent = nullptr);
    ~ObjectTreeModel() override = default;

    void setProject(Project* project);
    CustomObject* objectAt(const QModelIndex& index) const;
    CustomOperation* operationAt(const QModelIndex& index) const;

    // QAbstractItemModel interface
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Look up a tree model index for a given CustomObject pointer
    QModelIndex createIndexByObject(CustomObject* obj) const;

    // Internal node IDs — data, result, and operation children must be distinct
    static constexpr quintptr kDataFolderId   = 1;
    static constexpr quintptr kResultFolderId = 2;
    static constexpr quintptr kOpsFolderId    = 3;
    static constexpr quintptr kDataChildId    = 4;
    static constexpr quintptr kResultChildId  = 5;
    static constexpr quintptr kOpsChildId     = 6;

private:
    Project* m_project = nullptr;
    bool isFolderId(quintptr id) const {
        return id == kDataFolderId || id == kResultFolderId || id == kOpsFolderId;
    }
};

} // namespace ExpressDesigner