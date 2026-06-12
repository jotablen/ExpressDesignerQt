#pragma once
#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QWidget>
#include <core/Project.h>

namespace ExpressDesigner {

class OffsetWFDialog : public QDialog {
    Q_OBJECT
public:
    explicit OffsetWFDialog(QWidget* parent = nullptr);
    void setProject(Project* project);

    QComboBox* wfCombo() const { return m_wfCombo; }
    QLineEdit* offsetEdit() const { return m_offsetEdit; }
    QLineEdit* resultNameEdit() const { return m_resultNameEdit; }
    bool createNewResult() const { return m_newResultCheck->isChecked(); }

private slots:
    void onNewResultToggled(bool checked);
    void onWfChanged();

private:
    void populateCombo(Project* project);
    void updateResultName();

    QComboBox* m_wfCombo = nullptr;
    QLineEdit* m_offsetEdit = nullptr;
    QLineEdit* m_resultNameEdit = nullptr;
    QCheckBox* m_newResultCheck = nullptr;
    QWidget* m_resultPanel = nullptr;
    Project* m_project = nullptr;
};

} // namespace ExpressDesigner