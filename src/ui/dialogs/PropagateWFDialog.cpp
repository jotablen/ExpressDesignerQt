#include "PropagateWFDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QFrame>
#include <QMessageBox>

namespace ExpressDesigner {

PropagateWFDialog::PropagateWFDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Propagate Wavefront through surface"));

    auto* layout = new QVBoxLayout(this);

    // --- Wavefront selection ---
    auto* form = new QFormLayout();

    m_wfOrgCombo = new QComboBox(this);
    form->addRow(QStringLiteral("Wavefront being propagated:"), m_wfOrgCombo);

    m_wfDestCombo = new QComboBox(this);
    form->addRow(QStringLiteral("Surface to propagate over:"), m_wfDestCombo);

    layout->addLayout(form);

    auto* sep1 = new QFrame(this);
    sep1->setFrameShape(QFrame::HLine);
    sep1->setFrameShadow(QFrame::Sunken);
    layout->addWidget(sep1);

    // --- Parameters ---
    auto* form2 = new QFormLayout();

    m_amountEdit = new QLineEdit(QStringLiteral("100"), this);
    form2->addRow(QStringLiteral("Amount of control points:"), m_amountEdit);

    m_indexDestEdit = new QLineEdit(QStringLiteral("1.5"), this);
    form2->addRow(QStringLiteral("Next refractive index:"), m_indexDestEdit);

    m_offsetEdit = new QLineEdit(QStringLiteral("0"), this);
    form2->addRow(QStringLiteral("Offset after propagation:"), m_offsetEdit);

    layout->addLayout(form2);

    auto* sep2 = new QFrame(this);
    sep2->setFrameShape(QFrame::HLine);
    sep2->setFrameShadow(QFrame::Sunken);
    layout->addWidget(sep2);

    // --- Result name ---
    m_resultNameEdit = new QLineEdit(this);
    layout->addWidget(new QLabel(QStringLiteral("Result wave-front object name:"), this));
    layout->addWidget(m_resultNameEdit);

    auto* sep3 = new QFrame(this);
    sep3->setFrameShape(QFrame::HLine);
    sep3->setFrameShadow(QFrame::Sunken);
    layout->addWidget(sep3);

    // --- Exclude RTI checkbox (hidden) ---
    m_excludeRtiCheck = new QCheckBox(QStringLiteral("Exclude TI&R points from result"), this);
    m_excludeRtiCheck->setChecked(true);
    m_excludeRtiCheck->setVisible(false);
    layout->addWidget(m_excludeRtiCheck);

    // --- Buttons ---
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
        // Validation matching original PropagateSimple::btnOkClick
        if (m_wfOrgCombo->currentIndex() < 0 || m_wfDestCombo->currentIndex() < 0) {
            QMessageBox::critical(this, QStringLiteral("ERROR!!! Selecting WFs"),
                QStringLiteral("Please, to realize the calculation, you must \nselect both WF Objects (Origin and Destiny)."));
            m_wfDestCombo->setFocus();
            return;
        }
        if (m_resultNameEdit->text().trimmed().isEmpty()) {
            QMessageBox::critical(this, QStringLiteral("ERROR!!! Missing Object Name"),
                QStringLiteral("Please, enter a name to the \nResult Object."));
            m_resultNameEdit->setFocus();
            return;
        }
        bool ok = false;
        int amount = m_amountEdit->text().toInt(&ok);
        if (!ok || amount <= 0) {
            QMessageBox::critical(this, QStringLiteral("ERROR!!! Setting the amount of points"),
                QStringLiteral("Please, to calculate, \nenter only positives integer number."));
            m_amountEdit->setFocus();
            return;
        }
        double indexDest = m_indexDestEdit->text().toDouble(&ok);
        if (!ok || indexDest <= 0) {
            QMessageBox::critical(this, QStringLiteral("ERROR!!! Setting the refractive index"),
                QStringLiteral("Please, to calculate, \nenter only positive numbers."));
            m_indexDestEdit->setFocus();
            return;
        }
        m_offsetEdit->text().toDouble(&ok);
        if (!ok) {
            QMessageBox::critical(this, QStringLiteral("ERROR!!! Setting the offset after propagation"),
                QStringLiteral("Please, to calculate, \nenter only numbers."));
            m_offsetEdit->setFocus();
            return;
        }
        m_excludeRtiCheck->setChecked(true);
        accept();
    });

    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    connect(m_wfOrgCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PropagateWFDialog::onWfOrgChanged);
}

void PropagateWFDialog::setProject(Project* project)
{
    m_project = project;
    populateCombos(project);

    // Default selections matching FormShow
    if (m_wfOrgCombo->count() > 0)
        m_wfOrgCombo->setCurrentIndex(0);
    if (m_wfDestCombo->count() > 0)
        m_wfDestCombo->setCurrentIndex(0);

    // If a WF is pre-selected, find it in the combo and select it
    if (m_selectedWF) {
        int idx = m_wfOrgCombo->findText(m_selectedWF->name());
        if (idx >= 0)
            m_wfOrgCombo->setCurrentIndex(idx);
    }

    m_excludeRtiCheck->setChecked(true);
    updateResultName();
}

void PropagateWFDialog::setSelectedWF(CustomObject* wf)
{
    m_selectedWF = wf;
}

void PropagateWFDialog::populateCombos(Project* project)
{
    m_wfOrgCombo->clear();
    m_wfDestCombo->clear();

    if (!project) return;

    // Populate WF Origin (only wavefront objects)
    const auto& dataObjects = project->dataObjects();
    for (CustomObject* obj : dataObjects) {
        if (!obj) continue;
        if (isWavefront(obj->objectType())) {
            m_wfOrgCombo->addItem(obj->name(), QVariant::fromValue(reinterpret_cast<quintptr>(obj)));
        }
    }

    // Populate WF Dest (objects that are NOT WFs and NOT Points — surfaces only)
    for (CustomObject* obj : dataObjects) {
        if (!obj) continue;
        auto bt = toBaseType(obj->objectType());
        if (!isWavefront(obj->objectType()) && bt != 0x001)
            m_wfDestCombo->addItem(obj->name(), QVariant::fromValue(reinterpret_cast<quintptr>(obj)));
    }
    const auto& resultObjects = project->resultObjects();
    for (CustomObject* obj : resultObjects) {
        if (!obj) continue;
        auto bt = toBaseType(obj->objectType());
        if (!isWavefront(obj->objectType()) && bt != 0x001)
            m_wfDestCombo->addItem(obj->name(), QVariant::fromValue(reinterpret_cast<quintptr>(obj)));
    }
}

void PropagateWFDialog::onWfOrgChanged()
{
    updateResultName();
}

void PropagateWFDialog::updateResultName()
{
    if (m_wfOrgCombo->currentIndex() >= 0) {
        QString name = m_wfOrgCombo->currentText() + QStringLiteral(" Propg");
        m_resultNameEdit->setText(name);
    }
}

} // namespace ExpressDesigner