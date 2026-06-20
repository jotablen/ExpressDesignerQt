#include "TranslateObjectDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>

namespace ExpressDesigner {

TranslateObjectDialog::TranslateObjectDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle(tr("Translate Object(s)"));
    auto* layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel(tr("Select object(s) to translate:"), this));
    m_objectList = new QListWidget(this);
    m_objectList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    layout->addWidget(m_objectList);

    auto* form = new QFormLayout();
    m_deltaXEdit = new QLineEdit(QStringLiteral("0"), this);
    form->addRow(tr("Delta &X:"), m_deltaXEdit);
    m_deltaYEdit = new QLineEdit(QStringLiteral("0"), this);
    form->addRow(tr("Delta &Y:"), m_deltaYEdit);
    layout->addLayout(form);

    auto* btnLayout = new QHBoxLayout();
    m_okBtn = new QPushButton(tr("&Translate"), this);
    m_okBtn->setDefault(true);
    m_okBtn->setEnabled(false);
    auto* cancelBtn = new QPushButton(tr("Cancel"), this);
    btnLayout->addStretch();
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(m_okBtn);
    layout->addLayout(btnLayout);

    connect(m_okBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_objectList, &QListWidget::itemSelectionChanged, this, [this]{
        m_okBtn->setEnabled(!m_objectList->selectedItems().isEmpty());
    });
}

void TranslateObjectDialog::setProject(Project* project)
{
    m_objectList->clear();
    if (!project) return;
    for (auto* obj : project->allObjects())
        m_objectList->addItem(obj->name());
}

void TranslateObjectDialog::setSelectedObject(CustomObject* obj)
{
    if (!obj) return;
    for (int i = 0; i < m_objectList->count(); ++i) {
        if (m_objectList->item(i)->text() == obj->name()) {
            m_objectList->item(i)->setSelected(true);
            break;
        }
    }
}

} // namespace ExpressDesigner