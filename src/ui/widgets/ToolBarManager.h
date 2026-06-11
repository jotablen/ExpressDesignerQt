#pragma once
#include <QObject>
class QToolBar;

namespace ExpressDesigner {

class ToolBarManager : public QObject {
    Q_OBJECT
public:
    explicit ToolBarManager(QObject* parent = nullptr) : QObject(parent) {}
    void setupToolBar(QToolBar* toolbar) { Q_UNUSED(toolbar); }
};

} // namespace ExpressDesigner