#pragma once
#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <core/Project.h>

namespace ExpressDesigner {
class ExportObjectDialog : public QDialog {
    Q_OBJECT
public:
    explicit ExportObjectDialog(QWidget* parent = nullptr);
    void setProject(Project* project);
    QString fileName() const { return m_fileNameEdit->text(); }
    bool exchangeYZ() const { return m_exchangeYzCheck->isChecked(); }
private slots:
    void onBrowse();
    void onMethodChanged(int index);
private:
    void populateObjects(Project* project);
    QComboBox* m_objectCombo = nullptr;
    QComboBox* m_methodCombo = nullptr;
    QLineEdit* m_fileNameEdit = nullptr;
    QCheckBox* m_exchangeYzCheck = nullptr;
};
}