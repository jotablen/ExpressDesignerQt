#pragma once

#include <QDialog>
#include <QProgressBar>
#include <QLabel>
#include <QTimer>

namespace ExpressDesigner {

class SplashDialog : public QDialog {
    Q_OBJECT
public:
    explicit SplashDialog(QWidget* parent = nullptr);
    ~SplashDialog() override = default;

    void setMessage(const QString& msg);

private:
    QLabel* m_label = nullptr;
    QProgressBar* m_progress = nullptr;
};

} // namespace ExpressDesigner