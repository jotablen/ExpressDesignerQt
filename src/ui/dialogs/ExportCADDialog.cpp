#include "ExportCADDialog.h"
#include <core/ObjectTypes.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QFileDialog>
#include <QMessageBox>

namespace ExpressDesigner {

ExportCADDialog::ExportCADDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Export CAD (Step/IGES)"));
    resize(780, 520);

    auto* mainLayout = new QHBoxLayout(this);

    // ═══════════════════ Left panel — object list ═══════════════════
    auto* leftPanel = new QVBoxLayout();

    auto* leftLabel = new QLabel(QStringLiteral("Select objects to export:"), this);
    leftLabel->setStyleSheet(QStringLiteral("font-weight: bold;"));
    leftPanel->addWidget(leftLabel);

    m_objectList = new QListWidget(this);
    m_objectList->setSelectionMode(QAbstractItemView::MultiSelection);
    leftPanel->addWidget(m_objectList, 1);

    m_includeWFsCheck = new QCheckBox(QStringLiteral("Include WFs (wavefronts)"), this);
    m_includeWFsCheck->setChecked(false);
    leftPanel->addWidget(m_includeWFsCheck);

    mainLayout->addLayout(leftPanel, 1);

    // ═══════════════════ Right panel — options ═══════════════════
    auto* rightPanel = new QVBoxLayout();

    // ── Wires only / File ──
    m_wiresOnlyCheck = new QCheckBox(QStringLiteral("Export only wires (no faces / no solids)"), this);
    rightPanel->addWidget(m_wiresOnlyCheck);

    auto* sep0 = new QFrame(this);
    sep0->setFrameShape(QFrame::HLine);
    sep0->setFrameShadow(QFrame::Sunken);
    rightPanel->addWidget(sep0);

    // ── Rotational extrusion ──
    auto* rotGroup = new QGroupBox(QStringLiteral("Rotational extrusion"));
    auto* rotLayout = new QVBoxLayout(rotGroup);
    m_rotationalCheck = new QCheckBox(QStringLiteral("Enable"));
    rotLayout->addWidget(m_rotationalCheck);

    auto* rotForm = new QFormLayout();
    m_rotAxisCombo = new QComboBox();
    m_rotAxisCombo->addItems({QStringLiteral("X"), QStringLiteral("Y"), QStringLiteral("Z")});
    m_rotAxisCombo->setCurrentIndex(1);
    m_rotAxisCombo->setEnabled(false);
    rotForm->addRow(QStringLiteral("Axis:"), m_rotAxisCombo);

    m_rotAngleStartSpin = new QDoubleSpinBox();
    m_rotAngleStartSpin->setRange(-360.0, 360.0);
    m_rotAngleStartSpin->setDecimals(2);
    m_rotAngleStartSpin->setValue(0.0);
    m_rotAngleStartSpin->setSuffix(QStringLiteral(" °"));
    m_rotAngleStartSpin->setEnabled(false);
    rotForm->addRow(QStringLiteral("Angle start:"), m_rotAngleStartSpin);

    m_rotAngleEndSpin = new QDoubleSpinBox();
    m_rotAngleEndSpin->setRange(-360.0, 360.0);
    m_rotAngleEndSpin->setDecimals(2);
    m_rotAngleEndSpin->setValue(360.0);
    m_rotAngleEndSpin->setSuffix(QStringLiteral(" °"));
    m_rotAngleEndSpin->setEnabled(false);
    rotForm->addRow(QStringLiteral("Angle end:"), m_rotAngleEndSpin);

    m_rotAngularStepsSpin = new QDoubleSpinBox();
    m_rotAngularStepsSpin->setRange(1, 3600);
    m_rotAngularStepsSpin->setDecimals(0);
    m_rotAngularStepsSpin->setValue(36);
    m_rotAngularStepsSpin->setEnabled(false);
    rotForm->addRow(QStringLiteral("Angular steps:"), m_rotAngularStepsSpin);

    rotLayout->addLayout(rotForm);
    rightPanel->addWidget(rotGroup);

    // ── Linear extrusion ──
    auto* linGroup = new QGroupBox(QStringLiteral("Linear extrusion"));
    auto* linLayout = new QVBoxLayout(linGroup);
    m_linearCheck = new QCheckBox(QStringLiteral("Enable"));
    linLayout->addWidget(m_linearCheck);

    auto* linForm = new QFormLayout();
    m_linearDirCombo = new QComboBox();
    m_linearDirCombo->addItems({QStringLiteral("X"), QStringLiteral("Y"), QStringLiteral("Z")});
    m_linearDirCombo->setCurrentIndex(2);
    m_linearDirCombo->setEnabled(false);
    linForm->addRow(QStringLiteral("Direction:"), m_linearDirCombo);

    m_linearWidenessSpin = new QDoubleSpinBox();
    m_linearWidenessSpin->setRange(0.0, 10000.0);
    m_linearWidenessSpin->setDecimals(3);
    m_linearWidenessSpin->setValue(1.0);
    m_linearWidenessSpin->setSuffix(QStringLiteral(" mm"));
    m_linearWidenessSpin->setEnabled(false);
    linForm->addRow(QStringLiteral("Wideness:"), m_linearWidenessSpin);

    linLayout->addLayout(linForm);
    rightPanel->addWidget(linGroup);

    // ── File path ──
    auto* sep1 = new QFrame(this);
    sep1->setFrameShape(QFrame::HLine);
    sep1->setFrameShadow(QFrame::Sunken);
    rightPanel->addWidget(sep1);

    auto* fileRow = new QHBoxLayout();
    m_fileNameEdit = new QLineEdit(this);
    m_fileNameEdit->setPlaceholderText(QStringLiteral("Select output file..."));
    fileRow->addWidget(new QLabel(QStringLiteral("File:"), this));
    fileRow->addWidget(m_fileNameEdit);
    auto* browseBtn = new QPushButton(QStringLiteral("Browse..."));
    browseBtn->setFixedWidth(80);
    fileRow->addWidget(browseBtn);
    rightPanel->addLayout(fileRow);

    rightPanel->addStretch();

    // ── Export button ──
    m_exportButton = new QPushButton(QStringLiteral("Export"), this);
    m_exportButton->setEnabled(false);
    m_exportButton->setStyleSheet(QStringLiteral("QPushButton { font-weight: bold; padding: 8px 24px; }"));
    rightPanel->addWidget(m_exportButton);

    mainLayout->addLayout(rightPanel, 1);

    // ── Connections ──
    connect(m_objectList, &QListWidget::itemSelectionChanged, this, &ExportCADDialog::onSelectionChanged);
    connect(m_includeWFsCheck, &QCheckBox::toggled, this, &ExportCADDialog::onIncludeWFsToggled);
    connect(m_rotationalCheck, &QCheckBox::toggled, this, &ExportCADDialog::onRotationalToggled);
    connect(m_linearCheck, &QCheckBox::toggled, this, &ExportCADDialog::onLinearToggled);
    connect(browseBtn, &QPushButton::clicked, this, &ExportCADDialog::onBrowse);
    connect(m_exportButton, &QPushButton::clicked, this, [this]() {
        if (m_fileNameEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, tr("No file"), tr("Please choose an output file."));
            return;
        }
        if (selectedObjectNames().isEmpty()) {
            QMessageBox::warning(this, tr("No objects"), tr("Please select at least one object."));
            return;
        }
        accept();
    });
}

void ExportCADDialog::setProject(Project* project)
{
    m_project = project;
    populateObjects(project);
}

void ExportCADDialog::populateObjects(Project* project)
{
    m_objectList->clear();
    if (!project) return;

    // Always show non-WF data objects and result objects
    for (auto* obj : project->dataObjects()) {
        if (!obj) continue;
        if (!isWavefront(obj->objectType())
            && (obj->objectType() != ObjectType::Point))
            m_objectList->addItem(obj->name());
    }
    for (auto* obj : project->resultObjects()) {
        if (!obj) continue;
        if (!isWavefront(obj->objectType())
            && (obj->objectType() != ObjectType::Point))
            m_objectList->addItem(obj->name());
    }
    updateExportButton();
}

QStringList ExportCADDialog::selectedObjectNames() const
{
    QStringList names;
    const auto items = m_objectList->selectedItems();
    for (auto* item : items)
        names << item->text();
    return names;
}

void ExportCADDialog::refilterObjects()
{
    if (!m_project) return;
    const bool includeWFs = m_includeWFsCheck->isChecked();
    // Remember current selection names
    QStringList oldSelected = selectedObjectNames();

    m_objectList->clear();

    for (auto* obj : m_project->dataObjects()) {
        if (!obj) continue;
        if (isWavefront(obj->objectType()) && !includeWFs) continue;
        if (obj->objectType() == ObjectType::Point) continue;
        m_objectList->addItem(obj->name());
    }
    for (auto* obj : m_project->resultObjects()) {
        if (!obj) continue;
        if (isWavefront(obj->objectType()) && !includeWFs) continue;
        if (obj->objectType() == ObjectType::Point) continue;
        m_objectList->addItem(obj->name());
    }

    // Restore selection where possible
    for (int i = 0; i < m_objectList->count(); ++i) {
        if (oldSelected.contains(m_objectList->item(i)->text()))
            m_objectList->item(i)->setSelected(true);
    }
}

void ExportCADDialog::updateExportButton()
{
    m_exportButton->setEnabled(!selectedObjectNames().isEmpty());
}

void ExportCADDialog::onSelectionChanged()
{
    updateExportButton();
}

void ExportCADDialog::onIncludeWFsToggled(bool)
{
    refilterObjects();
}

void ExportCADDialog::onBrowse()
{
    QString path = QFileDialog::getSaveFileName(
        this,
        QStringLiteral("Export CAD File"),
        m_fileNameEdit->text().isEmpty() ? QStringLiteral("export.step") : m_fileNameEdit->text(),
        QStringLiteral("STEP Files (*.step *.stp);;IGES Files (*.igs *.iges);;All Files (*)"));
    if (!path.isEmpty())
        m_fileNameEdit->setText(path);
}

QString ExportCADDialog::rotationalAxis() const { return m_rotAxisCombo->currentText(); }
QString ExportCADDialog::linearDirection() const { return m_linearDirCombo->currentText(); }

void ExportCADDialog::onRotationalToggled(bool checked)
{
    m_rotAxisCombo->setEnabled(checked);
    m_rotAngleStartSpin->setEnabled(checked);
    m_rotAngleEndSpin->setEnabled(checked);
    m_rotAngularStepsSpin->setEnabled(checked);
}

void ExportCADDialog::onLinearToggled(bool checked)
{
    m_linearDirCombo->setEnabled(checked);
    m_linearWidenessSpin->setEnabled(checked);
}

} // namespace ExpressDesigner