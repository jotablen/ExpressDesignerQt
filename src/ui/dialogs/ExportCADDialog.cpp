#include "ExportCADDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
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
    auto* mainLayout = new QVBoxLayout(this);

    // ── Object selection ──
    auto* objForm = new QFormLayout();
    m_objectCombo = new QComboBox(this);
    objForm->addRow(QStringLiteral("Object:"), m_objectCombo);
    mainLayout->addLayout(objForm);

    // ── Wires-only option ──
    m_wiresOnlyCheck = new QCheckBox(QStringLiteral("Export only wires (no faces / no solids)"), this);
    mainLayout->addWidget(m_wiresOnlyCheck);

    // ── Separator ──
    auto* sep1 = new QFrame(this);
    sep1->setFrameShape(QFrame::HLine);
    sep1->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(sep1);

    // ── Rotational extrusion group ──
    auto* rotGroup = new QGroupBox(QStringLiteral("Rotational extrusion"), this);
    auto* rotLayout = new QVBoxLayout(rotGroup);
    m_rotationalCheck = new QCheckBox(QStringLiteral("Enable rotational extrusion"), this);
    rotLayout->addWidget(m_rotationalCheck);

    auto* rotForm = new QFormLayout();

    m_rotAxisCombo = new QComboBox(this);
    m_rotAxisCombo->addItems({QStringLiteral("X"), QStringLiteral("Y"), QStringLiteral("Z")});
    m_rotAxisCombo->setCurrentIndex(1); // Y axis by default
    m_rotAxisCombo->setEnabled(false);
    rotForm->addRow(QStringLiteral("Axis:"), m_rotAxisCombo);

    m_rotAngleStartSpin = new QDoubleSpinBox(this);
    m_rotAngleStartSpin->setRange(-360.0, 360.0);
    m_rotAngleStartSpin->setDecimals(2);
    m_rotAngleStartSpin->setValue(0.0);
    m_rotAngleStartSpin->setSuffix(QStringLiteral(" °"));
    m_rotAngleStartSpin->setEnabled(false);
    rotForm->addRow(QStringLiteral("Angle start:"), m_rotAngleStartSpin);

    m_rotAngleEndSpin = new QDoubleSpinBox(this);
    m_rotAngleEndSpin->setRange(-360.0, 360.0);
    m_rotAngleEndSpin->setDecimals(2);
    m_rotAngleEndSpin->setValue(360.0);
    m_rotAngleEndSpin->setSuffix(QStringLiteral(" °"));
    m_rotAngleEndSpin->setEnabled(false);
    rotForm->addRow(QStringLiteral("Angle end:"), m_rotAngleEndSpin);

    m_rotAngularStepsSpin = new QDoubleSpinBox(this);
    m_rotAngularStepsSpin->setRange(1, 3600);
    m_rotAngularStepsSpin->setDecimals(0);
    m_rotAngularStepsSpin->setValue(36);
    m_rotAngularStepsSpin->setEnabled(false);
    rotForm->addRow(QStringLiteral("Angular steps:"), m_rotAngularStepsSpin);

    rotLayout->addLayout(rotForm);
    mainLayout->addWidget(rotGroup);

    // ── Linear extrusion group ──
    auto* linGroup = new QGroupBox(QStringLiteral("Linear extrusion"), this);
    auto* linLayout = new QVBoxLayout(linGroup);
    m_linearCheck = new QCheckBox(QStringLiteral("Enable linear extrusion"), this);
    linLayout->addWidget(m_linearCheck);

    auto* linForm = new QFormLayout();

    m_linearDirCombo = new QComboBox(this);
    m_linearDirCombo->addItems({QStringLiteral("X"), QStringLiteral("Y"), QStringLiteral("Z")});
    m_linearDirCombo->setCurrentIndex(2); // Z direction by default
    m_linearDirCombo->setEnabled(false);
    linForm->addRow(QStringLiteral("Direction:"), m_linearDirCombo);

    m_linearWidenessSpin = new QDoubleSpinBox(this);
    m_linearWidenessSpin->setRange(0.0, 10000.0);
    m_linearWidenessSpin->setDecimals(3);
    m_linearWidenessSpin->setValue(1.0);
    m_linearWidenessSpin->setSuffix(QStringLiteral(" mm"));
    m_linearWidenessSpin->setEnabled(false);
    linForm->addRow(QStringLiteral("Wideness:"), m_linearWidenessSpin);

    linLayout->addLayout(linForm);
    mainLayout->addWidget(linGroup);

    // ── File path ──
    auto* sep2 = new QFrame(this);
    sep2->setFrameShape(QFrame::HLine);
    sep2->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(sep2);

    auto* fileRow = new QHBoxLayout();
    m_fileNameEdit = new QLineEdit(this);
    m_fileNameEdit->setPlaceholderText(QStringLiteral("Select output file..."));
    fileRow->addWidget(new QLabel(QStringLiteral("File:"), this));
    fileRow->addWidget(m_fileNameEdit);
    auto* browseBtn = new QPushButton(QStringLiteral("Browse..."), this);
    browseBtn->setFixedWidth(80);
    fileRow->addWidget(browseBtn);
    mainLayout->addLayout(fileRow);

    // ── OK / Cancel ──
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(buttons);

    // ── Connections ──
    connect(browseBtn, &QPushButton::clicked, this, &ExportCADDialog::onBrowse);
    connect(m_rotationalCheck, &QCheckBox::toggled, this, &ExportCADDialog::onRotationalToggled);
    connect(m_linearCheck, &QCheckBox::toggled, this, &ExportCADDialog::onLinearToggled);
    connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
        if (m_fileNameEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, tr("No file"), tr("Please choose an output file."));
            return;
        }
        accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void ExportCADDialog::setProject(Project* project) { populateObjects(project); }

void ExportCADDialog::populateObjects(Project* project)
{
    m_objectCombo->clear();
    if (!project) return;
    for (auto* obj : project->dataObjects()) {
        if (obj) m_objectCombo->addItem(obj->name());
    }
    for (auto* obj : project->resultObjects()) {
        if (obj) m_objectCombo->addItem(obj->name());
    }
    if (m_objectCombo->count() > 0)
        m_objectCombo->setCurrentIndex(0);
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