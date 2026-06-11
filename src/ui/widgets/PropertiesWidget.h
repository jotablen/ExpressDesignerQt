#pragma once
#include <QWidget>
#include <core/CustomObject.h>
#include <core/Project.h>

namespace ExpressDesigner {

class PropertiesWidget : public QWidget {
    Q_OBJECT
public:
    explicit PropertiesWidget(QWidget* parent = nullptr) : QWidget(parent) {}
    void setObject(CustomObject* obj) { Q_UNUSED(obj); }
    void setProject(Project* project) { Q_UNUSED(project); }
    void refresh() {}
};

} // namespace ExpressDesigner