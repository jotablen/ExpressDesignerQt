#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QGroupBox>

namespace ExpressDesigner {
class SetScaleDialog : public QDialog {
    Q_OBJECT
public:
    explicit SetScaleDialog(QWidget* parent = nullptr);
    double xMin() const;
    double xMax() const;
    double yMin() const;
    double yMax() const;
    void setRanges(double xmin, double xmax, double ymin, double ymax);
private:
    QLineEdit* m_xMinEdit = nullptr;
    QLineEdit* m_xMaxEdit = nullptr;
    QLineEdit* m_yMinEdit = nullptr;
    QLineEdit* m_yMaxEdit = nullptr;
};
}