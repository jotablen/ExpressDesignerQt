#pragma once
#include <QAbstractItemModel>
#include <core/Project.h>
#include <core/CustomObject.h>

namespace ExpressDesigner {

class ObjectTreeModel : public QAbstractItemModel {
    Q_OBJECT
public:
    explicit ObjectTreeModel(QObject* parent = nullptr);
    ~ObjectTreeModel() override = default;

    void setProject(Project* project);
    CustomObject* objectAt(const QModelIndex& index) const;

    // QAbstractItemModel interface
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    Project* m_project = nullptr;
    enum NodeType { RootNode, DataFolder, ResultFolder, ObjectNode };
    QModelIndex createIndexByObject(CustomObject* obj) const;
};

} // namespace ExpressDesigner