#include "EditPointDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QFrame>

namespace ExpressDesigner {
EditPointDialog::EditPointDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(QStringLiteral("Edit point coordinates... "));
    auto* layout = new QVBoxLayout(this);
    auto* panel = new QFrame(this);
    panel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    auto* form = new QFormLayout(panel);
    m_xEdit = new QLineEdit(QStringLiteral("0.0"), panel);
    form->addRow(QStringLiteral("&X coordinate:"), m_xEdit);
    m_yEdit = new QLineEdit(QStringLiteral("0.0"), panel);
    form->addRow(QStringLiteral("&Y coordinate:"), m_yEdit);
    layout->addWidget(panel);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}
void EditPointDialog::setCoordinates(double x, double y) { m_xEdit->setText(QString::number(x, 'f', 6)); m_yEdit->setText(QString::number(y, 'f', 6)); }
double EditPointDialog::x() const { return m_xEdit->text().toDouble(); }
double EditPointDialog::y() const { return m_yEdit->text().toDouble(); }
}