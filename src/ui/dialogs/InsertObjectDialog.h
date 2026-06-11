#pragma once
#include <QDialog>
#include <core/CustomObject.h>

namespace ExpressDesigner {

class InsertObjectDialog : public QDialog {
    Q_OBJECT
public:
    explicit InsertObjectDialog(QWidget* parent = nullptr);
    ~InsertObjectDialog() override = default;
    CustomObject* createdObject() const { return m_createdObject; }

private:
    CustomObject* m_createdObject = nullptr;
};

} // namespace ExpressDesigner