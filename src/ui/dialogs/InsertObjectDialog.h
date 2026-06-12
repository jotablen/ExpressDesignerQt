#pragma once
#include <QDialog>
#include <core/CustomObject.h>
#include <core/ObjectTypes.h>

class QComboBox;
class QLineEdit;
class QCheckBox;

namespace ExpressDesigner {

class InsertObjectDialog : public QDialog {
    Q_OBJECT
public:
    explicit InsertObjectDialog(QWidget* parent = nullptr);
    ~InsertObjectDialog() override = default;
    CustomObject* createdObject() const { return m_createdObject; }

private slots:
    void onIsWFChanged(bool checked);

private:
    void createObject();

    QComboBox* m_typeCombo = nullptr;
    QLineEdit* m_nameEdit = nullptr;
    QCheckBox* m_isWfCheck = nullptr;
    QCheckBox* m_isVirtualWfCheck = nullptr;
    CustomObject* m_createdObject = nullptr;
};

} // namespace ExpressDesigner