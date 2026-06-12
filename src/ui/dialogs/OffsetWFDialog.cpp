#include "OffsetWFDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QFrame>
#include <QMessageBox>

namespace ExpressDesigner {

OffsetWFDialog::OffsetWFDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Offset Wavefront dialog..."));

    auto* layout = new QVBoxLayout(this);

    // --- Selection row ---
    auto* selLayout = new QHBoxLayout();
    selLayout->addWidget(new QLabel(QStringLiteral("Wavefront being shifted:"), this));

    m_wfCombo = new QComboBox(this);
    m_wfCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    selLayout->addWidget(m_wfCombo);

    selLayout->addWidget(new QLabel(QStringLiteral("Offset distance:"), this));

    m_offsetEdit = new QLineEdit(QStringLiteral("1"), this);
    m_offsetEdit->setFixedWidth(120);
    selLayout->addWidget(m_offsetEdit);

    layout->addLayout(selLayout);

    auto* sep1 = new QFrame(this);
    sep1->setFrameShape(QFrame::HLine);
    sep1->setFrameShadow(QFrame::Sunken);
    layout->addWidget(sep1);

    // --- New result checkbox ---
    m_newResultCheck = new QCheckBox(QStringLiteral("Create new result object"), this);
    layout->addWidget(m_newResultCheck);

    // --- Result panel ---
    m_resultPanel = new QWidget(this);
    auto* resultLayout = new QVBoxLayout(m_resultPanel);
    resultLayout->setContentsMargins(0, 0, 0, 0);
    resultLayout->addWidget(new QLabel(QStringLiteral("Result wavefront object name:"), m_resultPanel));
    m_resultNameEdit = new QLineEdit(m_resultPanel);
    resultLayout->addWidget(m_resultNameEdit);
    m_resultPanel->setVisible(false);
    layout->addWidget(m_resultPanel);

    layout->addStretch();

    // --- Buttons ---
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
        if (m_newResultCheck->isChecked() && m_resultNameEdit->text().trimmed().isEmpty()) {
            QMessageBox::critical(this, QStringLiteral("ERROR!!! Missing Object Name"),
                QStringLiteral("Please, enter a name for the Result Object."));
            m_resultNameEdit->setFocus();
            return;
        }
        bool ok = false;
        m_offsetEdit->text().toDouble(&ok);
        if (!ok) {
            QMessageBox::critical(this, QStringLiteral("ERROR!!!"),
                QStringLiteral("Offset distance must be a number."));
            m_offsetEdit->setFocus();
            return;
        }
        accept();
    });

    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    connect(m_newResultCheck, &QCheckBox::toggled, this, &OffsetWFDialog::onNewResultToggled);
    connect(m_wfCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &OffsetWFDialog::onWfChanged);
}

void OffsetWFDialog::setProject(Project* project)
{
    m_project = project;
    populateCombo(project);

    if (m_wfCombo->count() > 0)
        m_wfCombo->setCurrentIndex(0);
}

void OffsetWFDialog::populateCombo(Project* project)
{
    m_wfCombo->clear();
    if (!project) return;

    const auto& dataObjects = project->dataObjects();
    for (CustomObject* obj : dataObjects) {
        if (!obj) continue;
        if (isWavefront(obj->objectType())) {
            m_wfCombo->addItem(obj->name(), QVariant::fromValue(reinterpret_cast<quintptr>(obj)));
        }
    }
    const auto& resultObjects = project->resultObjects();
    for (CustomObject* obj : resultObjects) {
        if (!obj) continue;
        if (isWavefront(obj->objectType())) {
            m_wfCombo->addItem(obj->name(), QVariant::fromValue(reinterpret_cast<quintptr>(obj)));
        }
    }
}

void OffsetWFDialog::onNewResultToggled(bool checked)
{
    m_resultPanel->setVisible(checked);
}

void OffsetWFDialog::onWfChanged()
{
    updateResultName();
}

void OffsetWFDialog::updateResultName()
{
    if (m_wfCombo->currentIndex() >= 0) {
        m_resultNameEdit->setText(m_wfCombo->currentText() + QStringLiteral("_offset"));
    }
}

} // namespace ExpressDesigner