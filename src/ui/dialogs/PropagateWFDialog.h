#pragma once
#include <QDialog>
#include <core/Project.h>

namespace ExpressDesigner {
class PropagateWFDialog : public QDialog {
    Q_OBJECT
public:
    explicit PropagateWFDialog(QWidget* parent = nullptr) : QDialog(parent) {}
    void setProject(Project* project) { Q_UNUSED(project); }
};
}