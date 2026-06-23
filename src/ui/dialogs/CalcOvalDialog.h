#pragma once
#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <core/Project.h>

namespace ExpressDesigner {

class CalcOvalDialog : public QDialog {
    Q_OBJECT
public:
    explicit CalcOvalDialog(QWidget* parent = nullptr);
    void setProject(Project* project);

    QComboBox* wfOriginCombo() const { return m_wfOrgCombo; }
    QComboBox* wfDestCombo() const { return m_wfDestCombo; }
    QComboBox* refPointCombo() const { return m_refPointCombo; }
    QLineEdit* amountEdit() const { return m_amountEdit; }
    QLineEdit* resultNameEdit() const { return m_resultNameEdit; }
    bool excludeRTI() const { return m_excludeRtiCheck->isChecked(); }
    int refPointKind() const;
    QString refPointSourceName() const;

private slots:
    void onWfOrgChanged();
    void onWfDestChanged();
    void updateResultName();

private:
    void populateCombos(Project* project);

    QComboBox* m_wfOrgCombo = nullptr;
    QComboBox* m_wfDestCombo = nullptr;
    QComboBox* m_refPointCombo = nullptr;
    QLineEdit* m_amountEdit = nullptr;
    QLineEdit* m_resultNameEdit = nullptr;
    QCheckBox* m_excludeRtiCheck = nullptr;
    Project* m_project = nullptr;
};

} // namespace ExpressDesigner
