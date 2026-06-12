#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QWidget>

namespace ExpressDesigner {
class NormalsParamsDialog : public QDialog {
    Q_OBJECT
public:
    explicit NormalsParamsDialog(QWidget* parent = nullptr);
    bool showNormals() const { return m_showNormalsCheck->isChecked(); }
    int amount() const { return m_amountEdit->text().toInt(); }
    double length() const { return m_lengthEdit->text().toDouble(); }
private slots:
    void onShowNormalsToggled(bool checked);
private:
    QCheckBox* m_showNormalsCheck = nullptr;
    QWidget* m_dataPanel = nullptr;
    QLineEdit* m_amountEdit = nullptr;
    QLineEdit* m_lengthEdit = nullptr;
};
}