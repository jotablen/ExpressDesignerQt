#pragma once
#include <QDialog>
class QLabel;
namespace ExpressDesigner {
class AboutDialog : public QDialog {
    Q_OBJECT
public:
    explicit AboutDialog(QWidget* parent = nullptr);
};
}