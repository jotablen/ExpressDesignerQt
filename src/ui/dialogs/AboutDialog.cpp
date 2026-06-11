#include "AboutDialog.h"
#include <QVBoxLayout>
#include <QLabel>

namespace ExpressDesigner {

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("About ExpressDesigner"));
    auto* layout = new QVBoxLayout(this);
    auto* label = new QLabel(QStringLiteral("ExpressDesigner v4.0.0\n\nQt6 + SISL Optical Design Tool"), this);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
}

} // namespace ExpressDesigner