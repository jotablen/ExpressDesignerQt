#include "SplashDialog.h"
#include <QVBoxLayout>

namespace ExpressDesigner {

SplashDialog::SplashDialog(QWidget* parent)
    : QDialog(parent, Qt::FramelessWindowHint)
{
    setFixedSize(400, 200);
    auto* layout = new QVBoxLayout(this);
    m_label = new QLabel(QStringLiteral("ExpressDesigner"), this);
    m_label->setAlignment(Qt::AlignCenter);
    m_progress = new QProgressBar(this);
    m_progress->setRange(0, 0);
    layout->addStretch();
    layout->addWidget(m_label);
    layout->addWidget(m_progress);
    layout->addStretch();
}

void SplashDialog::setMessage(const QString& msg) { m_label->setText(msg); }

} // namespace ExpressDesigner