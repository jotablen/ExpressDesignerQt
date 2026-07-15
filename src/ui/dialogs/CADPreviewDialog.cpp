#include "CADPreviewDialog.h"
#include <core/CADGeometryBuilder.h>
#include <io/CADExporter.h>
#include <utils/Logger.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QGroupBox>
#include <QFrame>
#include <QApplication>

#include <BRep_Builder.hxx>
#include <TopoDS_Compound.hxx>

namespace ExpressDesigner {

CADPreviewDialog::CADPreviewDialog(const CADExportParams& initialParams,
                                   QWidget* parent)
    : QDialog(parent)
    , m_params(initialParams)
{
    setWindowTitle(tr("CAD Preview — Raytracing"));
    resize(1200, 800);
    setupUi();
    buildAndDisplayShape();
}

CADPreviewDialog::~CADPreviewDialog() = default;

CADExportParams CADPreviewDialog::exportParams() const
{
    return m_params;
}

// ============================================================================
// setupUi – orchestrates sub-methods
// ============================================================================
void CADPreviewDialog::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    auto* splitter = new QSplitter(Qt::Horizontal, this);

    // Left: 3D Preview
    m_previewWidget = new CADPreviewWidget(splitter);
    splitter->addWidget(m_previewWidget);

    // Right: Controls panel
    auto* rightPanel = new QWidget(splitter);
    auto* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setSpacing(4);

    setupExtrusionGroup(rightPanel);
    setupDisplayGroup(rightPanel);
    setupRaytracingGroup(rightPanel);
    setupButtons(rightPanel);

    splitter->addWidget(rightPanel);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);
    mainLayout->addWidget(splitter);

    setupConnections();
}

// ============================================================================
// setupExtrusionGroup
// ============================================================================
void CADPreviewDialog::setupExtrusionGroup(QWidget* parent)
{
    auto* group = new QGroupBox(tr("Extrusion"), parent);
    auto* layout = new QVBoxLayout(group);

    m_rotationalCheck = new QCheckBox(tr("Rotational"), group);
    m_rotationalCheck->setChecked(m_params.rotational);
    layout->addWidget(m_rotationalCheck);

    auto* rotForm = new QFormLayout();
    m_rotAxisCombo = new QComboBox(group);
    m_rotAxisCombo->addItems({QStringLiteral("X"), QStringLiteral("Y"), QStringLiteral("Z")});
    m_rotAxisCombo->setCurrentText(m_params.rotationalAxis);
    rotForm->addRow(tr("Axis:"), m_rotAxisCombo);

    m_rotAngleStartSpin = new QDoubleSpinBox(group);
    m_rotAngleStartSpin->setRange(0, 360);
    m_rotAngleStartSpin->setValue(m_params.angleStart);
    m_rotAngleStartSpin->setSuffix(QStringLiteral("°"));
    rotForm->addRow(tr("Start:"), m_rotAngleStartSpin);

    m_rotAngleEndSpin = new QDoubleSpinBox(group);
    m_rotAngleEndSpin->setRange(0, 360);
    m_rotAngleEndSpin->setValue(m_params.angleEnd);
    m_rotAngleEndSpin->setSuffix(QStringLiteral("°"));
    rotForm->addRow(tr("End:"), m_rotAngleEndSpin);

    m_rotStepsSpin = new QDoubleSpinBox(group);
    m_rotStepsSpin->setRange(4, 360);
    m_rotStepsSpin->setValue(m_params.angularSteps);
    m_rotStepsSpin->setDecimals(0);
    rotForm->addRow(tr("Steps:"), m_rotStepsSpin);
    layout->addLayout(rotForm);

    m_linearCheck = new QCheckBox(tr("Linear"), group);
    m_linearCheck->setChecked(m_params.linear);
    layout->addWidget(m_linearCheck);

    auto* linForm = new QFormLayout();
    m_linearDirCombo = new QComboBox(group);
    m_linearDirCombo->addItems({QStringLiteral("X"), QStringLiteral("Y"), QStringLiteral("Z")});
    m_linearDirCombo->setCurrentText(m_params.linearDirection);
    linForm->addRow(tr("Dir:"), m_linearDirCombo);

    m_linearWidenessSpin = new QDoubleSpinBox(group);
    m_linearWidenessSpin->setRange(0.01, 1000);
    m_linearWidenessSpin->setValue(m_params.wideness);
    m_linearWidenessSpin->setSuffix(QStringLiteral(" mm"));
    linForm->addRow(tr("Depth:"), m_linearWidenessSpin);
    layout->addLayout(linForm);

    parent->layout()->addWidget(group);
}

// ============================================================================
// setupDisplayGroup
// ============================================================================
void CADPreviewDialog::setupDisplayGroup(QWidget* parent)
{
    auto* group = new QGroupBox(tr("Display / Material"), parent);
    auto* layout = new QVBoxLayout(group);

    m_wiresOnlyCheck = new QCheckBox(tr("Wires Only"), group);
    m_wiresOnlyCheck->setChecked(m_params.wiresOnly);
    layout->addWidget(m_wiresOnlyCheck);

    m_shadingCheck = new QCheckBox(tr("Shaded"), group);
    m_shadingCheck->setChecked(true);
    layout->addWidget(m_shadingCheck);

    auto* matForm = new QFormLayout();
    m_materialCombo = new QComboBox(group);
    m_materialCombo->addItems({
        QStringLiteral("Steel"), QStringLiteral("Aluminium"),
        QStringLiteral("Gold"), QStringLiteral("Copper"),
        QStringLiteral("Mirror"), QStringLiteral("Glass"), QStringLiteral("Plastic")
    });
    matForm->addRow(tr("Material:"), m_materialCombo);
    layout->addLayout(matForm);

    m_raytracingCheck = new QCheckBox(tr("Raytracing (GPU)"), group);
    m_raytracingCheck->setChecked(false);
    m_raytracingCheck->setToolTip(
        tr("Requires OpenGL 4.3+ GPU. Enables reflections, refractions, shadows."));
    layout->addWidget(m_raytracingCheck);

    parent->layout()->addWidget(group);
}

// ============================================================================
// setupRaytracingGroup
// ============================================================================
void CADPreviewDialog::setupRaytracingGroup(QWidget* parent)
{
    auto* group = new QGroupBox(tr("Raytracing Settings"), parent);
    auto* layout = new QFormLayout(group);

    m_reflBouncesSpin = new QSpinBox(group);
    m_reflBouncesSpin->setRange(0, 10);
    m_reflBouncesSpin->setValue(6);
    layout->addRow(tr("Refl. Bounces:"), m_reflBouncesSpin);

    m_refrBouncesSpin = new QSpinBox(group);
    m_refrBouncesSpin->setRange(0, 10);
    m_refrBouncesSpin->setValue(4);
    layout->addRow(tr("Refr. Bounces:"), m_refrBouncesSpin);

    m_shadowSoftnessSpin = new QDoubleSpinBox(group);
    m_shadowSoftnessSpin->setRange(0, 1);
    m_shadowSoftnessSpin->setSingleStep(0.1);
    m_shadowSoftnessSpin->setValue(0.5);
    layout->addRow(tr("Shadow Soft:"), m_shadowSoftnessSpin);

    m_shadowsCheck = new QCheckBox(tr("Shadows"), group);
    m_shadowsCheck->setChecked(true);
    layout->addRow(m_shadowsCheck);

    m_reflectionsCheck = new QCheckBox(tr("Reflections"), group);
    m_reflectionsCheck->setChecked(true);
    layout->addRow(m_reflectionsCheck);

    m_refractionsCheck = new QCheckBox(tr("Refractions"), group);
    m_refractionsCheck->setChecked(true);
    layout->addRow(m_refractionsCheck);

    m_envIntensitySpin = new QDoubleSpinBox(group);
    m_envIntensitySpin->setRange(0, 1);
    m_envIntensitySpin->setSingleStep(0.05);
    m_envIntensitySpin->setValue(0.3);
    layout->addRow(tr("Env Intensity:"), m_envIntensitySpin);

    parent->layout()->addWidget(group);
}

// ============================================================================
// setupButtons
// ============================================================================
void CADPreviewDialog::setupButtons(QWidget* parent)
{
    auto* layout = qobject_cast<QVBoxLayout*>(parent->layout());
    if (!layout) return;

    layout->addStretch();

    m_applyButton = new QPushButton(tr("Apply"), parent);
    m_applyButton->setDefault(true);
    layout->addWidget(m_applyButton);

    m_exportButton = new QPushButton(tr("Export to CAD..."), parent);
    layout->addWidget(m_exportButton);

    m_snapshotButton = new QPushButton(tr("Save Snapshot..."), parent);
    layout->addWidget(m_snapshotButton);

    m_resetViewButton = new QPushButton(tr("Reset View"), parent);
    layout->addWidget(m_resetViewButton);

    m_closeButton = new QPushButton(tr("Close"), parent);
    layout->addWidget(m_closeButton);
}

// ============================================================================
// setupConnections
// ============================================================================
void CADPreviewDialog::setupConnections()
{
    connect(m_applyButton, &QPushButton::clicked,
            this, &CADPreviewDialog::onApplyExtrusion);
    connect(m_shadingCheck, &QCheckBox::toggled,
            this, &CADPreviewDialog::onShadingToggled);
    connect(m_raytracingCheck, &QCheckBox::toggled,
            this, &CADPreviewDialog::onRaytracingToggled);
    connect(m_materialCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CADPreviewDialog::onMaterialChanged);
    connect(m_reflBouncesSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &CADPreviewDialog::onReflBouncesChanged);
    connect(m_refrBouncesSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &CADPreviewDialog::onRefrBouncesChanged);
    connect(m_shadowSoftnessSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &CADPreviewDialog::onShadowSoftnessChanged);
    connect(m_shadowsCheck, &QCheckBox::toggled,
            this, &CADPreviewDialog::onShadowsToggled);
    connect(m_reflectionsCheck, &QCheckBox::toggled,
            this, &CADPreviewDialog::onReflectionsToggled);
    connect(m_refractionsCheck, &QCheckBox::toggled,
            this, &CADPreviewDialog::onRefractionsToggled);
    connect(m_envIntensitySpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &CADPreviewDialog::onEnvIntensityChanged);
    connect(m_exportButton, &QPushButton::clicked,
            this, &CADPreviewDialog::onExportCAD);
    connect(m_snapshotButton, &QPushButton::clicked,
            this, &CADPreviewDialog::onSnapshot);
    connect(m_resetViewButton, &QPushButton::clicked,
            this, &CADPreviewDialog::onResetView);
    connect(m_closeButton, &QPushButton::clicked,
            this, &QDialog::reject);
}

// ============================================================================
// buildAndDisplayShape – router
// ============================================================================
void CADPreviewDialog::buildAndDisplayShape()
{
    const bool multiShape = !m_params.controlPointsList.isEmpty();
    TopoDS_Shape shape;

    if (multiShape)
        shape = buildCompoundFromList();
    else
        shape = buildSingleShape();

    if (shape.IsNull()) {
        m_previewWidget->clearShape();
        return;
    }
    m_previewWidget->setShape(shape);
}

// ============================================================================
// buildCompoundFromList – iterates over controlPointsList
// ============================================================================
TopoDS_Shape CADPreviewDialog::buildCompoundFromList()
{
    const int count = m_params.controlPointsList.size();
    LOG_INFO("CAD", QString("Building preview from %1 curves...").arg(count));

    TopoDS_Compound compound;
    BRep_Builder builder;
    builder.MakeCompound(compound);

    int okCount = 0;
    int failCount = 0;

    for (int i = 0; i < count; ++i) {
        const QVector<QPointF>& pts = m_params.controlPointsList[i];
        if (pts.size() < 2) {
            ++failCount;
            continue;
        }

        // Set per-object controlPoints so buildShapeRotational uses them
        m_params.controlPoints = pts;

        TopoDS_Wire wire;
        QString err;
        if (!CADGeometryBuilder::buildWire(pts, wire, err)) {
            ++failCount;
            LOG_WARN("CAD", QString("Wire[%1] build failed: %2").arg(i).arg(err));
            continue;
        }

        TopoDS_Shape shape;
        bool ok = CADGeometryBuilder::buildShape(m_params, wire, shape, err);
        if (!ok || shape.IsNull()) {
            ++failCount;
            LOG_WARN("CAD", QString("Shape[%1] build warning: %2").arg(i).arg(err));
            if (!wire.IsNull()) {
                builder.Add(compound, wire);
                ++okCount;
            }
            continue;
        }
        builder.Add(compound, shape);
        ++okCount;
    }

    if (okCount == 0) {
        LOG_WARN("CAD", "No shapes built successfully.");
        return TopoDS_Shape();
    }
    LOG_INFO("CAD", QString("Compound built: %1 shapes OK, %2 skipped")
             .arg(okCount).arg(failCount));
    return compound;
}

// ============================================================================
// buildSingleShape – fallback for single controlPoints
// ============================================================================
TopoDS_Shape CADPreviewDialog::buildSingleShape()
{
    if (m_params.controlPoints.size() < 2) {
        LOG_WARN("CAD", "Not enough control points for preview.");
        return TopoDS_Shape();
    }
    TopoDS_Wire wire;
    QString err;
    if (!CADGeometryBuilder::buildWire(m_params.controlPoints, wire, err)) {
        LOG_WARN("CAD", QString("Wire build failed: %1").arg(err));
        return TopoDS_Shape();
    }
    TopoDS_Shape shape;
    bool ok = CADGeometryBuilder::buildShape(m_params, wire, shape, err);
    if (!ok || shape.IsNull()) {
        LOG_WARN("CAD", QString("Shape build warning: %1").arg(err));
    }
    if (!shape.IsNull()) {
        LOG_INFO("CAD", QString("Preview shape built (%1 pts, rotational=%2, linear=%3)")
                 .arg(m_params.controlPoints.size())
                 .arg(m_params.rotational ? "Y" : "N")
                 .arg(m_params.linear ? "Y" : "N"));
    }
    return shape;
}

// ============================================================================
// Slots
// ============================================================================
void CADPreviewDialog::onApplyExtrusion()
{
    m_params.rotational = m_rotationalCheck->isChecked();
    m_params.rotationalAxis = m_rotAxisCombo->currentText();
    m_params.angleStart = m_rotAngleStartSpin->value();
    m_params.angleEnd = m_rotAngleEndSpin->value();
    m_params.angularSteps = static_cast<int>(m_rotStepsSpin->value());
    m_params.linear = m_linearCheck->isChecked();
    m_params.linearDirection = m_linearDirCombo->currentText();
    m_params.wideness = m_linearWidenessSpin->value();
    m_params.wiresOnly = m_wiresOnlyCheck->isChecked();
    buildAndDisplayShape();
}

void CADPreviewDialog::onShadingToggled(bool shaded)
{
    m_previewWidget->setShadingMode(shaded);
}

void CADPreviewDialog::onRaytracingToggled(bool enabled)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_previewWidget->setRaytracingEnabled(enabled);
    QApplication::restoreOverrideCursor();
}

void CADPreviewDialog::onMaterialChanged(int index)
{
    static const char* presets[] = {
        "steel", "aluminium", "gold", "copper",
        "mirror", "glass", "plastic"
    };
    QString preset = (index >= 0 && index < 7) ? QString(presets[index]) : QStringLiteral("steel");
    m_previewWidget->setMaterialPreset(preset);
}

void CADPreviewDialog::onReflBouncesChanged(int value)
{
    m_previewWidget->setReflectionBounces(value);
}

void CADPreviewDialog::onRefrBouncesChanged(int value)
{
    m_previewWidget->setRefractionBounces(value);
}

void CADPreviewDialog::onShadowSoftnessChanged(double value)
{
    m_previewWidget->setShadowSoftness(value);
}

void CADPreviewDialog::onShadowsToggled(bool enabled)
{
    m_previewWidget->setShadowsEnabled(enabled);
}

void CADPreviewDialog::onReflectionsToggled(bool enabled)
{
    m_previewWidget->setReflectionsEnabled(enabled);
}

void CADPreviewDialog::onRefractionsToggled(bool enabled)
{
    m_previewWidget->setRefractionsEnabled(enabled);
}

void CADPreviewDialog::onEnvIntensityChanged(double value)
{
    m_previewWidget->setEnvironmentIntensity(value);
}

void CADPreviewDialog::onExportCAD()
{
    onApplyExtrusion();
    QString filePath = QFileDialog::getSaveFileName(this, tr("Export to CAD"),
        QString(), tr("CAD Files (*.step *.stp *.iges *.igs);;STEP (*.step *.stp);;IGES (*.iges *.igs)"));
    if (filePath.isEmpty()) return;

    CADExportParams exportParams = m_params;
    exportParams.filePath = filePath;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool ok = CADExporter::exportToCAD(exportParams);
    QApplication::restoreOverrideCursor();

    if (ok)
        QMessageBox::information(this, tr("Export Successful"),
            tr("CAD file exported successfully:\n%1").arg(filePath));
    else
        QMessageBox::warning(this, tr("Export Failed"),
            tr("Failed to export CAD file:\n%1").arg(CADExporter::errorMessage()));
}

void CADPreviewDialog::onSnapshot()
{
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save Snapshot"),
        QStringLiteral("snapshot.png"),
        tr("PNG Images (*.png);;JPEG (*.jpg *.jpeg);;BMP (*.bmp)"));
    if (filePath.isEmpty()) return;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool ok = m_previewWidget->saveSnapshot(filePath, 1920, 1080);
    QApplication::restoreOverrideCursor();

    if (ok)
        QMessageBox::information(this, tr("Snapshot Saved"),
            tr("Snapshot saved to:\n%1").arg(filePath));
    else
        QMessageBox::warning(this, tr("Snapshot Failed"),
            tr("Could not save snapshot."));
}

void CADPreviewDialog::onResetView()
{
    buildAndDisplayShape();
}

} // namespace ExpressDesigner