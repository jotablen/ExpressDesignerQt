#pragma once
#include <QDialog>
#include <core/Project.h>

namespace ExpressDesigner {
class CalcOvalDialog : public QDialog {
    Q_OBJECT
public:
    explicit CalcOvalDialog(QWidget* parent = nullptr) : QDialog(parent) {}
    void setProject(Project* project) { Q_UNUSED(project); }
};
}