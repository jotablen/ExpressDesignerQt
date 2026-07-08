#pragma once
#include <QOpenGLWidget>
#include <QMouseEvent>
#include <QWheelEvent>
#include <TopoDS_Shape.hxx>
#include <AIS_InteractiveContext.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_View.hxx>
#include <Graphic3d_RenderingParams.hxx>
#include <Graphic3d_NameOfMaterial.hxx>
#include <Graphic3d_TextureEnv.hxx>
#include <Aspect_DisplayConnection.hxx>

namespace ExpressDesigner {

/**
 * @brief OpenGL widget for full raytracing CAD preview (Nivel 3).
 *
 * All features from Nivel 1 +:
 * - Raytracing with configurable bounces and samples
 * - PBR materials: Steel, Aluminium, Gold, Copper, Mirror, Glass, Plastic
 * - HDRI environment map (procedural sky gradient)
 * - Directional + positional lights
 * - Snapshot to QImage
 * - Post-processing tone mapping
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

    // ── Nivel 3: Raytracing ──
    void setRaytracingEnabled(bool enabled);

    /// Set material preset
    void setMaterialPreset(const QString& preset);

    /// Set raytracing reflection bounces (0-10)
    void setReflectionBounces(int bounces);

    /// Set raytracing refraction bounces (0-10)
    void setRefractionBounces(int bounces);

    /// Set shadow softness (0=hard, 1=softest)
    void setShadowSoftness(double softness);

    /// Enable/disable shadows
    void setShadowsEnabled(bool enabled);

    /// Enable/disable reflections
    void setReflectionsEnabled(bool enabled);

    /// Enable/disable refractions
    void setRefractionsEnabled(bool enabled);

    /// Set environment map intensity (0.0 - 1.0)
    void setEnvironmentIntensity(double intensity);

    /// Setup a procedural HDRI sky gradient
    void setupProceduralHDRI(const QColor& skyTop, const QColor& skyBottom);

    /// Take a high-resolution snapshot and save to file
    bool saveSnapshot(const QString& filePath, int width = 1920, int height = 1080);

    /// Get the rendering params for fine-tuning
    Graphic3d_RenderingParams& renderingParams();

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
    void applyRaytracingParams();
    void setupDefaultLights();
    void fitAll();

    Handle(V3d_Viewer) m_viewer;
    Handle(V3d_View) m_view;
    Handle(AIS_InteractiveContext) m_context;
    Handle(AIS_Shape) m_aisShape;
    Handle(Aspect_DisplayConnection) m_displayConnection;
    Handle(Graphic3d_TextureEnv) m_envTexture;

    TopoDS_Shape m_currentShape;
    bool m_shaded = true;
    bool m_raytracing = false;
    bool m_viewerInitialized = false;

    // Material
    QString m_materialPreset = QStringLiteral("steel");

    // Raytracing parameters
    int m_reflectionBounces = 6;
    int m_refractionBounces = 4;
    double m_shadowSoftness = 0.5;
    bool m_shadowsEnabled = true;
    bool m_reflectionsEnabled = true;
    bool m_refractionsEnabled = true;
    double m_envIntensity = 0.3;

    // Mouse state
    Qt::MouseButtons m_mouseButtons = Qt::NoButton;
    QPoint m_lastPos;
};

} // namespace ExpressDesigner