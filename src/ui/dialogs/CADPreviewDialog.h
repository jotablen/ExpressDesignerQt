#pragma once
#include <QDialog>
#include <QPushButton>
#include <QCheckBox>
#include <QSplitter>
#include <QGroupBox>
#include <QDoubleSpinBox>
#include <TopoDS_Shape.hxx>
#include <core/CADGeometryBuilder.h>
#include <ui/widgets/CADPreviewWidget.h>

namespace ExpressDesigner {

/**
 * @brief Dialog for interactive 3D preview of CAD extrusions (Nivel 1).
 *
 * Allows the user to:
 * - Rotate, pan, zoom the 3D view
 * - Toggle rotational/linear extrusion
 * - Adjust extrusion parameters in real-time
 * - Toggle shading/wireframe
 * - Export to CAD
 *
 * Nivel 1 keeps it simple — no materials or raytracing.
 * See feature/preview_cad_raytracing for Nivel 3.
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
    void onExportCAD();
    void onResetView();

private:
    void setupUi();
    void buildAndDisplayShape();

    // Preview widget
    CADPreviewWidget* m_previewWidget = nullptr;

    // Parameters
    CADExportParams m_params;

    // Controls
    QCheckBox* m_rotationalCheck = nullptr;
    QComboBox* m_rotAxisCombo = nullptr;
    QDoubleSpinBox* m_rotAngleStartSpin = nullptr;
    QDoubleSpinBox* m_rotAngleEndSpin = nullptr;
    QDoubleSpinBox* m_rotStepsSpin = nullptr;

    QCheckBox* m_linearCheck = nullptr;
    QComboBox* m_linearDirCombo = nullptr;
    QDoubleSpinBox* m_linearWidenessSpin = nullptr;

    QCheckBox* m_wiresOnlyCheck = nullptr;
    QCheckBox* m_shadingCheck = nullptr;

    QPushButton* m_applyButton = nullptr;
    QPushButton* m_exportButton = nullptr;
    QPushButton* m_resetViewButton = nullptr;
    QPushButton* m_closeButton = nullptr;
};

} // namespace ExpressDesigner