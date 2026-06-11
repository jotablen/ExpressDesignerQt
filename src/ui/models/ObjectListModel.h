#pragma once
#include <QAbstractListModel>
#include <core/CustomObject.h>

namespace ExpressDesigner {

class ObjectListModel : public QAbstractListModel {
    Q_OBJECT
public:
    explicit ObjectListModel(QObject* parent = nullptr) : QAbstractListModel(parent) {}

    void setObjects(const QVector<CustomObject*>& objects) {
        beginResetModel();
        m_objects = objects;
        endResetModel();
    }

    int rowCount(const QModelIndex& = QModelIndex()) const override { return m_objects.size(); }
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
        if (role == Qt::DisplayRole && index.isValid())
            return m_objects.at(index.row())->name();
        return QVariant();
    }

private:
    QVector<CustomObject*> m_objects;
};

} // namespace ExpressDesigner