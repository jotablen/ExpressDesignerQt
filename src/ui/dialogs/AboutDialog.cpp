#include "AboutDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPixmap>
#include <QFrame>

namespace ExpressDesigner {

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("About ExpressDesigner"));
    setFixedSize(436, 440);

    auto* layout = new QVBoxLayout(this);

    auto* panel = new QFrame(this);
    panel->setFrameStyle(QFrame::Panel | QFrame::Raised);
    auto* panelLayout = new QVBoxLayout(panel);

    auto* imageLabel = new QLabel(panel);
    QPixmap aboutPixmap(":/resources/ExpressDesigner-About.png");
    if (!aboutPixmap.isNull()) {
        imageLabel->setPixmap(aboutPixmap.scaled(400, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    imageLabel->setAlignment(Qt::AlignCenter);
    panelLayout->addWidget(imageLabel);

    auto* separatorLine = new QFrame(panel);
    separatorLine->setFrameShape(QFrame::HLine);
    separatorLine->setFrameShadow(QFrame::Sunken);
    panelLayout->addWidget(separatorLine);

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
