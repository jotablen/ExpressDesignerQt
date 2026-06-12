#pragma once
#include <QDialog>
#include <QLineEdit>

namespace ExpressDesigner {
class EditPointDialog : public QDialog {
    Q_OBJECT
public:
    explicit EditPointDialog(QWidget* parent = nullptr);
    void setCoordinates(double x, double y);
    double x() const;
    double y() const;
private:
    QLineEdit* m_xEdit = nullptr;
    QLineEdit* m_yEdit = nullptr;
};
}