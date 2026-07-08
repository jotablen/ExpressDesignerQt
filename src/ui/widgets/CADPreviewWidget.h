#pragma once
#include <QOpenGLWidget>
#include <QMouseEvent>
#include <QWheelEvent>
#include <TopoDS_Shape.hxx>
#include <AIS_InteractiveContext.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_View.hxx>
#include <Graphic3d_RenderingParams.hxx>
#include <Aspect_DisplayConnection.hxx>

namespace ExpressDesigner {

/**
 * @brief OpenGL widget for interactive 3D preview of CAD shapes.
 *
 * Uses OpenCASCADE's Visualization module (AIS_InteractiveContext + V3d_View)
 * to display and manipulate TopoDS_Shape objects with mouse interaction.
 *
 * Supports:
 * - Pan (middle mouse button drag)
 * - Rotate (left mouse button drag)
 * - Zoom (mouse wheel)
 * - Shaded / Wireframe toggle
 * - PBR materials (steel/aluminium/gold/glass)
 * - Optional raytracing
 *
 * Note: Uses QOpenGLWidget::winId() after widget is shown to create
 * the OCCT window handle (WNT_Window on Windows).
 */
class CADPreviewWidget : public QOpenGLWidget {
    Q_OBJECT
public:
    explicit CADPreviewWidget(QWidget* parent = nullptr);
    ~CADPreviewWidget() override;

    /// Set the shape to display (replaces any existing shape)
    void setShape(const TopoDS_Shape& shape);

    /// Clear the displayed shape
    void clearShape();

    /// Toggle shading mode (shaded vs wireframe)
    void setShadingMode(bool shaded);

    /// Toggle raytracing on/off
    void setRaytracingEnabled(bool enabled);

    /// Set material preset: "steel", "aluminium", "gold", "glass"
    void setMaterialPreset(const QString& preset);

    /// Take a snapshot of the current view and return as QImage
    QImage snapshot() const;

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    // Mouse interaction
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void initViewer();
    void updateMaterial();
    void fitAll();

    Handle(V3d_Viewer) m_viewer;
    Handle(V3d_View) m_view;
    Handle(AIS_InteractiveContext) m_context;
    Handle(AIS_Shape) m_aisShape;
    Handle(Aspect_DisplayConnection) m_displayConnection;

    TopoDS_Shape m_currentShape;
    bool m_shaded = true;
    bool m_raytracing = false;
    QString m_materialPreset = QStringLiteral("steel");
    bool m_viewerInitialized = false;

    // Mouse state
    Qt::MouseButtons m_mouseButtons = Qt::NoButton;
    QPoint m_lastPos;
};

} // namespace ExpressDesigner