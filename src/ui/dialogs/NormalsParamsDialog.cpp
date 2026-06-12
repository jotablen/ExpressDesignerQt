#include "NormalsParamsDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QFrame>
#include <QLabel>

namespace ExpressDesigner {
NormalsParamsDialog::NormalsParamsDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(QStringLiteral("Setting normals visibility parameters"));
    auto* layout = new QVBoxLayout(this);
    m_showNormalsCheck = new QCheckBox(QStringLiteral("Show WFs normals"), this);
    layout->addWidget(m_showNormalsCheck);
    auto* frame = new QFrame(this);
    frame->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    m_dataPanel = frame;
    auto* form = new QFormLayout(m_dataPanel);
    m_amountEdit = new QLineEdit(QStringLiteral("3"), m_dataPanel);
    form->addRow(QStringLiteral("Amount of normals:"), m_amountEdit);
    m_lengthEdit = new QLineEdit(QStringLiteral("1"), m_dataPanel);
    form->addRow(QStringLiteral("Length of normals:"), m_lengthEdit);
    layout->addWidget(m_dataPanel);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_showNormalsCheck, &QCheckBox::toggled, this, &NormalsParamsDialog::onShowNormalsToggled);
}
void NormalsParamsDialog::onShowNormalsToggled(bool checked) { m_dataPanel->setEnabled(checked); }
}