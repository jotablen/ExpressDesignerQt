#pragma once
#include <QDialog>
#include <QComboBox>
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

    QComboBox* objectCombo() const { return m_objectCombo; }
    QLineEdit* degreesEdit() const { return m_degreesEdit; }
    int pivotMode() const; // 0=StartPoint, 1=MidPoint, 2=EndPoint

private slots:
    void onObjectChanged();

private:
    void populateCombo(Project* project);

    QComboBox* m_objectCombo = nullptr;
    QLineEdit* m_degreesEdit = nullptr;
    QButtonGroup* m_pivotGroup = nullptr;
    QRadioButton* m_pivotStart = nullptr;
    QRadioButton* m_pivotMid = nullptr;
    QRadioButton* m_pivotEnd = nullptr;
    Project* m_project = nullptr;
    CustomObject* m_selectedObj = nullptr;
};

} // namespace ExpressDesigner