#include "AboutDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QFrame>

namespace ExpressDesigner {

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("About"));
    setFixedSize(436, 343);

    auto* layout = new QVBoxLayout(this);

    auto* panel = new QFrame(this);
    panel->setFrameStyle(QFrame::Panel | QFrame::Raised);
    auto* panelLayout = new QVBoxLayout(panel);

    auto* titleLabel = new QLabel(QStringLiteral("Ovals Designer"), panel);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);

    auto* versionLabel = new QLabel(QStringLiteral("Version 4.0.0"), panel);
    versionLabel->setAlignment(Qt::AlignCenter);

    auto* copyLabel = new QLabel(QStringLiteral("Copyright \u00A9 2024 J. Blen"), panel);
    copyLabel->setAlignment(Qt::AlignCenter);

    auto* commentLabel = new QLabel(QStringLiteral("Qt6 + SISL Optical Design Tool\n\nOptical Surface Designer"), panel);
    commentLabel->setAlignment(Qt::AlignCenter);
    commentLabel->setWordWrap(true);

    panelLayout->addStretch();
    panelLayout->addWidget(titleLabel);
    panelLayout->addWidget(versionLabel);
    panelLayout->addWidget(copyLabel);
    panelLayout->addWidget(commentLabel);
    panelLayout->addStretch();

    layout->addWidget(panel);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
}

} // namespace ExpressDesigner
