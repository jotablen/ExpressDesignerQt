#pragma once
#include <QDialog>
#include <QListWidget>
#include <QLineEdit>
#include <QButtonGroup>
#include <QRadioButton>
#include <core/Project.h>

namespace ExpressDesigner {

class RotateObjectDialog : public QDialog {
    Q_OBJECT
public:
    explicit RotateObjectDialog(QWidget* parent = nullptr);
    void setProject(Project* project);
    void setSelectedObject(CustomObject* obj);

    QListWidget* objectList() const { return m_objectList; }
    QLineEdit* degreesEdit() const { return m_degreesEdit; }
    int pivotMode() const; // 0=StartPoint, 1=MidPoint, 2=EndPoint

private:
    QListWidget* m_objectList = nullptr;
    QLineEdit* m_degreesEdit = nullptr;
    QPushButton* m_okBtn = nullptr;
    QButtonGroup* m_pivotGroup = nullptr;
    QRadioButton* m_pivotStart = nullptr;
    QRadioButton* m_pivotMid = nullptr;
    QRadioButton* m_pivotEnd = nullptr;
    Project* m_project = nullptr;
};

} // namespace ExpressDesigner