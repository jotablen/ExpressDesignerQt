#include "RotateObjectDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>

namespace ExpressDesigner {

RotateObjectDialog::RotateObjectDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle(tr("Rotate Object(s)"));
    auto* layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel(tr("Select object(s) to rotate:"), this));
    m_objectList = new QListWidget(this);
    m_objectList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    layout->addWidget(m_objectList);

    auto* degLayout = new QHBoxLayout();
    degLayout->addWidget(new QLabel(tr("Rotation degrees:"), this));
    m_degreesEdit = new QLineEdit(QStringLiteral("0"), this);
    degLayout->addWidget(m_degreesEdit);
    layout->addLayout(degLayout);

    auto* pivotGroup = new QGroupBox(tr("Pivot mode"), this);
    auto* pivotLayout = new QVBoxLayout(pivotGroup);
    m_pivotGroup = new QButtonGroup(this);
    m_pivotStart = new QRadioButton(tr("Start point"), pivotGroup);
    m_pivotMid = new QRadioButton(tr("Mid point"), pivotGroup);
    m_pivotEnd = new QRadioButton(tr("End point"), pivotGroup);
    m_pivotMid->setChecked(true);
    m_pivotGroup->addButton(m_pivotStart, 0);
    m_pivotGroup->addButton(m_pivotMid, 1);
    m_pivotGroup->addButton(m_pivotEnd, 2);
    pivotLayout->addWidget(m_pivotStart);
    pivotLayout->addWidget(m_pivotMid);
    pivotLayout->addWidget(m_pivotEnd);
    layout->addWidget(pivotGroup);

    auto* btnLayout = new QHBoxLayout();
    auto* okBtn = new QPushButton(tr("&Rotate"), this);
    okBtn->setDefault(true);
    auto* cancelBtn = new QPushButton(tr("Cancel"), this);
    btnLayout->addStretch();
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(okBtn);
    layout->addLayout(btnLayout);

    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void RotateObjectDialog::setProject(Project* project)
{
    m_project = project;
    m_objectList->clear();
    if (!project) return;
    for (auto* obj : project->allObjects())
        m_objectList->addItem(obj->name());
}

void RotateObjectDialog::setSelectedObject(CustomObject* obj)
{
    if (!obj) return;
    for (int i = 0; i < m_objectList->count(); ++i) {
        if (m_objectList->item(i)->text() == obj->name()) {
            m_objectList->item(i)->setSelected(true);
            break;
        }
    }
}

int RotateObjectDialog::pivotMode() const
{
    return m_pivotGroup ? m_pivotGroup->checkedId() : 1;
}

} // namespace ExpressDesigner