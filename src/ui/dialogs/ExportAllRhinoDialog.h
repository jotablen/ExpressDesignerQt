#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>

namespace ExpressDesigner {

class ExportAllRhinoDialog : public QDialog {
    Q_OBJECT
public:
    explicit ExportAllRhinoDialog(QWidget* parent = nullptr);

    QString fileName() const { return m_fileNameEdit->text(); }
    void setFileName(const QString& name) { m_fileNameEdit->setText(name); }
    bool includeWFs() const { return m_includeWfCheck->isChecked(); }
    bool exchangeYZ() const { return m_exchangeYzCheck->isChecked(); }

private slots:
    void onBrowse();

private:
    QLineEdit* m_fileNameEdit = nullptr;
    QCheckBox* m_includeWfCheck = nullptr;
    QCheckBox* m_exchangeYzCheck = nullptr;
};

} // namespace ExpressDesigner