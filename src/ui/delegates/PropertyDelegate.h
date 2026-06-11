#pragma once
#include <QStyledItemDelegate>

namespace ExpressDesigner {

class PropertyDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit PropertyDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}
};

} // namespace ExpressDesigner