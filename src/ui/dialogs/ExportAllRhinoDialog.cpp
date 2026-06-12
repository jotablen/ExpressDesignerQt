#include "ExportAllRhinoDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QFileDialog>

namespace ExpressDesigner {

ExportAllRhinoDialog::ExportAllRhinoDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Export all objects to Rhino command file"));

    auto* layout = new QVBoxLayout(this);

    // --- File selection ---
    auto* fileRow = new QHBoxLayout();
    fileRow->addWidget(new QLabel(QStringLiteral("Sa&ve project to file:"), this));
    m_fileNameEdit = new QLineEdit(this);
    fileRow->addWidget(m_fileNameEdit);

    auto* browseBtn = new QPushButton(QStringLiteral("Browse..."), this);
    browseBtn->setFixedWidth(80);
    fileRow->addWidget(browseBtn);
    layout->addLayout(fileRow);

    auto* sep1 = new QFrame(this);
    sep1->setFrameShape(QFrame::HLine);
    sep1->setFrameShadow(QFrame::Sunken);
    layout->addWidget(sep1);

    // --- Checkboxes ---
    m_includeWfCheck = new QCheckBox(QStringLiteral("Include WFs in the exported data"), this);
    layout->addWidget(m_includeWfCheck);

    m_exchangeYzCheck = new QCheckBox(QStringLiteral("Rotate exported objects (from 'XY' to 'XZ')"), this);
    layout->addWidget(m_exchangeYzCheck);

    auto* sep2 = new QFrame(this);
    sep2->setFrameShape(QFrame::HLine);
    sep2->setFrameShadow(QFrame::Sunken);
    layout->addWidget(sep2);

    // --- Buttons ---
    auto* buttons = new QDialogButtonBox(this);
    auto* acceptBtn = buttons->addButton(QStringLiteral("&Accept"), QDialogButtonBox::AcceptRole);
    auto* cancelBtn = buttons->addButton(QStringLiteral("&Cancel"), QDialogButtonBox::RejectRole);
    layout->addWidget(buttons);

    connect(acceptBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(browseBtn, &QPushButton::clicked, this, &ExportAllRhinoDialog::onBrowse);
}

void ExportAllRhinoDialog::onBrowse()
{
    QString path = QFileDialog::getSaveFileName(this,
        QStringLiteral("Export Rhino Script"),
        m_fileNameEdit->text(),
        QStringLiteral("Rhino Script (*.rvb);;All Files (*)"));
    if (!path.isEmpty())
        m_fileNameEdit->setText(path);
}

} // namespace ExpressDesigner