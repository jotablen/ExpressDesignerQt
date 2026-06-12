#include "SetScaleDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QLabel>

namespace ExpressDesigner {
SetScaleDialog::SetScaleDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(QStringLiteral("Setting Axis Scale Form"));
    auto* layout = new QVBoxLayout(this);

    auto* xGroup = new QGroupBox(QStringLiteral("&X Axis Scale"), this);
    auto* xForm = new QFormLayout(xGroup);
    m_xMinEdit = new QLineEdit(QStringLiteral("-15"), xGroup);
    xForm->addRow(QStringLiteral("M&in:"), m_xMinEdit);
    m_xMaxEdit = new QLineEdit(QStringLiteral("15"), xGroup);
    xForm->addRow(QStringLiteral("M&ax:"), m_xMaxEdit);
    layout->addWidget(xGroup);

    auto* yGroup = new QGroupBox(QStringLiteral("&Y Axis Scale"), this);
    auto* yForm = new QFormLayout(yGroup);
    m_yMinEdit = new QLineEdit(QStringLiteral("-15"), yGroup);
    yForm->addRow(QStringLiteral("&Min:"), m_yMinEdit);
    m_yMaxEdit = new QLineEdit(QStringLiteral("15"), yGroup);
    yForm->addRow(QStringLiteral("Ma&x:"), m_yMaxEdit);
    layout->addWidget(yGroup);

    auto* buttons = new QDialogButtonBox(this);
    auto* acceptBtn = buttons->addButton(QStringLiteral("Ace&ptar"), QDialogButtonBox::AcceptRole);
    auto* cancelBtn = buttons->addButton(QStringLiteral("&Cancelar"), QDialogButtonBox::RejectRole);
    layout->addWidget(buttons);
    connect(acceptBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

double SetScaleDialog::xMin() const { return m_xMinEdit->text().toDouble(); }
double SetScaleDialog::xMax() const { return m_xMaxEdit->text().toDouble(); }
double SetScaleDialog::yMin() const { return m_yMinEdit->text().toDouble(); }
double SetScaleDialog::yMax() const { return m_yMaxEdit->text().toDouble(); }

void SetScaleDialog::setRanges(double xmin, double xmax, double ymin, double ymax) {
    m_xMinEdit->setText(QString::number(xmin));
    m_xMaxEdit->setText(QString::number(xmax));
    m_yMinEdit->setText(QString::number(ymin));
    m_yMaxEdit->setText(QString::number(ymax));
}
}