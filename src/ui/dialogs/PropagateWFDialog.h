#pragma once
#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <core/Project.h>

namespace ExpressDesigner {

class PropagateWFDialog : public QDialog {
    Q_OBJECT
public:
    explicit PropagateWFDialog(QWidget* parent = nullptr);
    void setProject(Project* project);
    void setSelectedWF(CustomObject* wf);

    QComboBox* wfOrgCombo() const { return m_wfOrgCombo; }
    QComboBox* wfDestCombo() const { return m_wfDestCombo; }
    QLineEdit* amountEdit() const { return m_amountEdit; }
    QLineEdit* resultNameEdit() const { return m_resultNameEdit; }
    QLineEdit* indexDestEdit() const { return m_indexDestEdit; }
    QLineEdit* offsetEdit() const { return m_offsetEdit; }
    bool excludeRTI() const { return m_excludeRtiCheck->isChecked(); }

private slots:
    void onWfOrgChanged();

private:
    void populateCombos(Project* project);
    void updateResultName();

    QComboBox* m_wfOrgCombo = nullptr;
    QComboBox* m_wfDestCombo = nullptr;
    QLineEdit* m_amountEdit = nullptr;
    QLineEdit* m_resultNameEdit = nullptr;
    QLineEdit* m_indexDestEdit = nullptr;
    QLineEdit* m_offsetEdit = nullptr;
    QCheckBox* m_excludeRtiCheck = nullptr;
    Project* m_project = nullptr;
    CustomObject* m_selectedWF = nullptr;
};

} // namespace ExpressDesigner