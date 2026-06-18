#include "CopyObjectDialog.h"

namespace ExpressDesigner {

CopyObjectDialog::CopyObjectDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle(tr("Copy Object"));
    auto* layout = new QVBoxLayout(this);

    auto* form = new QFormLayout();
    m_objectCombo = new QComboBox(this);
    form->addRow(tr("&Object to copy:"), m_objectCombo);

    m_newNameEdit = new QLineEdit(this);
    form->addRow(tr("&New name:"), m_newNameEdit);
    layout->addLayout(form);

    auto* btnLayout = new QHBoxLayout();
    auto* okBtn = new QPushButton(tr("&Copy"), this);
    okBtn->setDefault(true);
    auto* cancelBtn = new QPushButton(tr("Cancel"), this);
    btnLayout->addStretch();
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(okBtn);
    layout->addLayout(btnLayout);

    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void CopyObjectDialog::setProject(Project* project)
{
    m_objectCombo->clear();
    if (!project) return;
    for (auto* obj : project->allObjects())
        m_objectCombo->addItem(obj->name());
    if (m_objectCombo->count() > 0)
        m_newNameEdit->setText(m_objectCombo->currentText() + QStringLiteral("_copy"));
}

QString CopyObjectDialog::sourceName() const { return m_objectCombo->currentText(); }
QString CopyObjectDialog::newName() const { return m_newNameEdit->text().trimmed(); }

} // namespace ExpressDesigner