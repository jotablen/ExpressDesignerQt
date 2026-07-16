#pragma once
#include <QDialog>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QSplitter>
#include <QGroupBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QSlider>
#include <QComboBox>
#include <QSettings>
#include <TopoDS_Shape.hxx>
#include <core/CADGeometryBuilder.h>
#include <ui/widgets/CADPreviewWidget.h>

namespace ExpressDesigner {

/**
 * @brief Dialog for interactive 3D preview of CAD extrusions (Nivel 3).
 *
 * Full raytracing with PBR materials, HDRI, lights, and snapshot.
 */
class CADPreviewDialog : public QDialog {
    Q_OBJECT
public:
    explicit CADPreviewDialog(const CADExportParams& initialParams,
                              QWidget* parent = nullptr);
    ~CADPreviewDialog() override;

    /// Returns the final CAD export parameters
    CADExportParams exportParams() const;

private slots:
    void onApplyExtrusion();
    void onShadingToggled(bool shaded);
    void onRaytracingToggled(bool enabled);
    void onMaterialChanged(int index);
    void onReflBouncesChanged(int value);
    void onRefrBouncesChanged(int value);
    void onShadowSoftnessChanged(double value);
    void onShadowsToggled(bool enabled);
    void onReflectionsToggled(bool enabled);
    void onRefractionsToggled(bool enabled);
    void onEnvIntensityChanged(double value);
    void onExportCAD();
    void onSnapshot();
    void onResetView();

private:
    void setupUi();
    void setupExtrusionGroup(QWidget* parent);
    void setupDisplayGroup(QWidget* parent);
    void setupRaytracingGroup(QWidget* parent);
    void setupButtons(QWidget* parent);
    void setupConnections();

    void saveSettings();
    void buildAndDisplayShape();
    TopoDS_Shape buildCompoundFromList();
    TopoDS_Shape buildSingleShape();

    // Preview widget
    CADPreviewWidget* m_previewWidget = nullptr;

    // Parameters
    CADExportParams m_params;

    // Extrusion controls
    QCheckBox* m_rotationalCheck = nullptr;
    QComboBox* m_rotAxisCombo = nullptr;
    QDoubleSpinBox* m_rotAngleStartSpin = nullptr;
    QDoubleSpinBox* m_rotAngleEndSpin = nullptr;
    QDoubleSpinBox* m_rotStepsSpin = nullptr;

    QCheckBox* m_linearCheck = nullptr;
    QComboBox* m_linearDirCombo = nullptr;
    QDoubleSpinBox* m_linearWidenessSpin = nullptr;
    QCheckBox* m_wiresOnlyCheck = nullptr;

    // Display controls
    QCheckBox* m_shadingCheck = nullptr;
    QCheckBox* m_raytracingCheck = nullptr;
    QComboBox* m_materialCombo = nullptr;

    // Raytracing advanced controls
    QSpinBox* m_reflBouncesSpin = nullptr;
    QSpinBox* m_refrBouncesSpin = nullptr;
    QDoubleSpinBox* m_shadowSoftnessSpin = nullptr;
    QCheckBox* m_shadowsCheck = nullptr;
    QCheckBox* m_reflectionsCheck = nullptr;
    QCheckBox* m_refractionsCheck = nullptr;
    QDoubleSpinBox* m_envIntensitySpin = nullptr;

    // Buttons
    QPushButton* m_applyButton = nullptr;
    QPushButton* m_exportButton = nullptr;
    QPushButton* m_snapshotButton = nullptr;
    QPushButton* m_resetViewButton = nullptr;
    QPushButton* m_closeButton = nullptr;
};

} // namespace ExpressDesigner