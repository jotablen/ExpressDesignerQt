#include "RotateObjectDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QFrame>
#include <QMessageBox>

namespace ExpressDesigner {

RotateObjectDialog::RotateObjectDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Rotate Object"));
    setMinimumWidth(400);

    auto* layout = new QVBoxLayout(this);

    // Object selection
    auto* selLayout = new QHBoxLayout();
    selLayout->addWidget(new QLabel(QStringLiteral("Object:"), this));
    m_objectCombo = new QComboBox(this);
    m_objectCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    selLayout->addWidget(m_objectCombo);
    layout->addLayout(selLayout);

    auto* sep1 = new QFrame(this);
    sep1->setFrameShape(QFrame::HLine);
    sep1->setFrameShadow(QFrame::Sunken);
    layout->addWidget(sep1);

    // Pivot selection
    layout->addWidget(new QLabel(QStringLiteral("Pivot point:"), this));
    auto* pivotLayout = new QVBoxLayout();
    m_pivotGroup = new QButtonGroup(this);
    m_pivotStart = new QRadioButton(QStringLiteral("Desde punto inicial"), this);
    m_pivotMid = new QRadioButton(QStringLiteral("Desde punto medio"), this);
    m_pivotEnd = new QRadioButton(QStringLiteral("Desde punto final"), this);
    m_pivotStart->setChecked(true);
    m_pivotGroup->addButton(m_pivotStart, 0);
    m_pivotGroup->addButton(m_pivotMid, 1);
    m_pivotGroup->addButton(m_pivotEnd, 2);
    pivotLayout->addWidget(m_pivotStart);
    pivotLayout->addWidget(m_pivotMid);
    pivotLayout->addWidget(m_pivotEnd);
    layout->addLayout(pivotLayout);

    auto* sep2 = new QFrame(this);
    sep2->setFrameShape(QFrame::HLine);
    sep2->setFrameShadow(QFrame::Sunken);
    layout->addWidget(sep2);

    // Degrees
    auto* degLayout = new QHBoxLayout();
    degLayout->addWidget(new QLabel(QStringLiteral("Grados de rotación:"), this));
    m_degreesEdit = new QLineEdit(QStringLiteral("0"), this);
    m_degreesEdit->setFixedWidth(100);
    degLayout->addWidget(m_degreesEdit);
    degLayout->addWidget(new QLabel(QStringLiteral("° (positivo = antihorario)"), this));
    layout->addLayout(degLayout);

    layout->addStretch();

    // Buttons
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
        if (m_objectCombo->currentIndex() < 0) {
            QMessageBox::critical(this, QStringLiteral("ERROR!!!"),
                QStringLiteral("Please select an object to rotate."));
            return;
        }
        bool ok = false;
        m_degreesEdit->text().toDouble(&ok);
        if (!ok) {
            QMessageBox::critical(this, QStringLiteral("ERROR!!!"),
                QStringLiteral("Degrees must be a number."));
            m_degreesEdit->setFocus();
            return;
        }
        accept();
    });

    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void RotateObjectDialog::setProject(Project* project)
{
    m_project = project;
    populateCombo(project);

    if (m_selectedObj) {
        int idx = m_objectCombo->findText(m_selectedObj->name());
        if (idx >= 0)
            m_objectCombo->setCurrentIndex(idx);
    } else if (m_objectCombo->count() > 0) {
        m_objectCombo->setCurrentIndex(0);
    }
}

void RotateObjectDialog::setSelectedObject(CustomObject* obj)
{
    m_selectedObj = obj;
}

int RotateObjectDialog::pivotMode() const
{
    return m_pivotGroup ? m_pivotGroup->checkedId() : 0;
}

void RotateObjectDialog::onObjectChanged() {}

void RotateObjectDialog::populateCombo(Project* project)
{
    m_objectCombo->clear();
    if (!project) return;

    for (auto* obj : project->dataObjects()) {
        if (!obj) continue;
        m_objectCombo->addItem(obj->name(), QVariant::fromValue(reinterpret_cast<quintptr>(obj)));
    }
    for (auto* obj : project->resultObjects()) {
        if (!obj) continue;
        m_objectCombo->addItem(obj->name(), QVariant::fromValue(reinterpret_cast<quintptr>(obj)));
    }
}

} // namespace ExpressDesigner