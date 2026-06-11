#pragma once
#include <QWidget>
#include <core/CustomObject.h>
#include <core/Project.h>

namespace ExpressDesigner {

class ObjectTreeWidget : public QWidget {
    Q_OBJECT
public:
    explicit ObjectTreeWidget(QWidget* parent = nullptr) : QWidget(parent) {}
    void setProject(Project* project) { Q_UNUSED(project); }
    void refresh() {}
};

} // namespace ExpressDesigner