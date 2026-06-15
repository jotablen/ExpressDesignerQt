#pragma once
#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <core/Project.h>

namespace ExpressDesigner {

class TranslateObjectDialog : public QDialog {
    Q_OBJECT
public:
    explicit TranslateObjectDialog(QWidget* parent = nullptr);
    void setProject(Project* project);
    void setSelectedObject(CustomObject* obj);

    QComboBox* objectCombo() const { return m_objectCombo; }
    QLineEdit* deltaXEdit() const { return m_deltaXEdit; }
    QLineEdit* deltaYEdit() const { return m_deltaYEdit; }

private:
    void populateCombo(Project* project);

    QComboBox* m_objectCombo = nullptr;
    QLineEdit* m_deltaXEdit = nullptr;
    QLineEdit* m_deltaYEdit = nullptr;
    Project* m_project = nullptr;
    CustomObject* m_selectedObj = nullptr;
};

} // namespace ExpressDesigner