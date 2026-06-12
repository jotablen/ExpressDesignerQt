#include "ImportObjectDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>

namespace ExpressDesigner {
ImportObjectDialog::ImportObjectDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(QStringLiteral("Import object dialog"));
    auto* layout = new QVBoxLayout(this);
    auto* form = new QFormLayout();
    m_methodCombo = new QComboBox(this);
    m_methodCombo->addItem(QStringLiteral("Ovals Designer object style"));
    m_methodCombo->addItem(QStringLiteral("Tab limited xyz points style (Curve Object)"));
    m_methodCombo->addItem(QStringLiteral("Rhino Export file style (Curve Object)"));
    m_methodCombo->addItem(QStringLiteral("Tab limited xyz points style (Point Object)"));
    m_methodCombo->addItem(QStringLiteral("Rhino script file style (Point Object)"));
    form->addRow(QStringLiteral("Select &import method:"), m_methodCombo);
    layout->addLayout(form);
    auto* fileRow = new QHBoxLayout();
    m_fileNameEdit = new QLineEdit(this);
    fileRow->addWidget(new QLabel(QStringLiteral("&File name:"), this));
    fileRow->addWidget(m_fileNameEdit);
    auto* browseBtn = new QPushButton(QStringLiteral("&Browse"), this);
    browseBtn->setFixedWidth(80);
    fileRow->addWidget(browseBtn);
    layout->addLayout(fileRow);
    auto* form2 = new QFormLayout();
    m_objectNameEdit = new QLineEdit(this);
    form2->addRow(QStringLiteral("Set new &object name:"), m_objectNameEdit);
    layout->addLayout(form2);
    m_isWfCheck = new QCheckBox(QStringLiteral("It's going to be a &WF object"), this);
    layout->addWidget(m_isWfCheck);
    auto* buttons = new QDialogButtonBox(this);
    auto* acceptBtn = buttons->addButton(QStringLiteral("&Accept"), QDialogButtonBox::AcceptRole);
    auto* cancelBtn = buttons->addButton(QStringLiteral("&Cancelar"), QDialogButtonBox::RejectRole);
    layout->addWidget(buttons);
    connect(acceptBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(browseBtn, &QPushButton::clicked, this, &ImportObjectDialog::onBrowse);
    connect(m_methodCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ImportObjectDialog::onMethodChanged);
}
void ImportObjectDialog::onBrowse() {
    QString path = QFileDialog::getOpenFileName(this, QStringLiteral("Import File"), m_fileNameEdit->text());
    if (!path.isEmpty()) m_fileNameEdit->setText(path);
}
void ImportObjectDialog::onMethodChanged(int) {}
}