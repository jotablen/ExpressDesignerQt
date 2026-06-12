#include "ExportObjectDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QFileDialog>

namespace ExpressDesigner {
ExportObjectDialog::ExportObjectDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Export Object"));
    auto* layout = new QVBoxLayout(this);
    auto* form = new QFormLayout();
    m_objectCombo = new QComboBox(this);
    form->addRow(QStringLiteral("Object:"), m_objectCombo);
    layout->addLayout(form);
    auto* form2 = new QFormLayout();
    m_methodCombo = new QComboBox(this);
    m_methodCombo->addItem(QStringLiteral("Ovals Designer object style"));
    m_methodCombo->addItem(QStringLiteral("Tab limited xyz points style"));
    m_methodCombo->addItem(QStringLiteral("Rhino script file style"));
    form2->addRow(QStringLiteral("Method:"), m_methodCombo);
    layout->addLayout(form2);
    auto* fileRow = new QHBoxLayout();
    m_fileNameEdit = new QLineEdit(this);
    fileRow->addWidget(new QLabel(QStringLiteral("File:"), this));
    fileRow->addWidget(m_fileNameEdit);
    auto* browseBtn = new QPushButton(QStringLiteral("Browse..."), this);
    browseBtn->setFixedWidth(80);
    fileRow->addWidget(browseBtn);
    layout->addLayout(fileRow);
    m_exchangeYzCheck = new QCheckBox(QStringLiteral("Rotate exported objects (from 'XY' to 'XZ')"), this);
    layout->addWidget(m_exchangeYzCheck);
    auto* sep = new QFrame(this);
    sep->setFrameShape(QFrame::HLine);
    sep->setFrameShadow(QFrame::Sunken);
    layout->addWidget(sep);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(browseBtn, &QPushButton::clicked, this, &ExportObjectDialog::onBrowse);
}
void ExportObjectDialog::setProject(Project* project) { populateObjects(project); }
void ExportObjectDialog::populateObjects(Project* project) {
    m_objectCombo->clear();
    if (!project) return;
    for (auto* obj : project->dataObjects()) { if (obj) m_objectCombo->addItem(obj->name()); }
    for (auto* obj : project->resultObjects()) { if (obj) m_objectCombo->addItem(obj->name()); }
    if (m_objectCombo->count() > 0) m_objectCombo->setCurrentIndex(0);
}
void ExportObjectDialog::onBrowse() {
    QString path = QFileDialog::getSaveFileName(this, QStringLiteral("Export File"), m_fileNameEdit->text());
    if (!path.isEmpty()) m_fileNameEdit->setText(path);
}
void ExportObjectDialog::onMethodChanged(int) {}
}