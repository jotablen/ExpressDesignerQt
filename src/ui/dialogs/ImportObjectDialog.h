#pragma once
#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>

namespace ExpressDesigner {
class ImportObjectDialog : public QDialog {
    Q_OBJECT
public:
    explicit ImportObjectDialog(QWidget* parent = nullptr);
    QComboBox* methodCombo() const { return m_methodCombo; }
    QLineEdit* fileNameEdit() const { return m_fileNameEdit; }
    QLineEdit* objectNameEdit() const { return m_objectNameEdit; }
    bool isWF() const { return m_isWfCheck->isChecked(); }
private slots:
    void onBrowse();
    void onMethodChanged(int index);
private:
    QComboBox* m_methodCombo = nullptr;
    QLineEdit* m_fileNameEdit = nullptr;
    QLineEdit* m_objectNameEdit = nullptr;
    QCheckBox* m_isWfCheck = nullptr;
};
}