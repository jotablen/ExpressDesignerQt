#pragma once
#include <QWidget>
#include <QMouseEvent>
#include <QWheelEvent>
#include <TopoDS_Shape.hxx>
#include <AIS_Shape.hxx>
#include <AIS_InteractiveContext.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_View.hxx>
#include <Graphic3d_RenderingParams.hxx>
#include <Graphic3d_NameOfMaterial.hxx>
#include <Graphic3d_TextureEnv.hxx>
#include <Aspect_DisplayConnection.hxx>

namespace ExpressDesigner {

class CADPreviewWidget : public QWidget {
    Q_OBJECT
public:
    explicit CADPreviewWidget(QWidget* parent = nullptr);
    ~CADPreviewWidget() override;

    void setShape(const TopoDS_Shape& shape);
    void clearShape();
    void setShadingMode(bool shaded);

    void setRaytracingEnabled(bool enabled);
    void setMaterialPreset(const QString& preset);
    void setReflectionBounces(int bounces);
    void setRefractionBounces(int bounces);
    void setShadowSoftness(double softness);
    void setShadowsEnabled(bool enabled);
    void setReflectionsEnabled(bool enabled);
    void setRefractionsEnabled(bool enabled);
    void setEnvironmentIntensity(double intensity);
    void setupProceduralHDRI(const QColor& skyTop, const QColor& skyBottom);
    bool saveSnapshot(const QString& filePath, int width = 1920, int height = 1080);
    Graphic3d_RenderingParams& renderingParams();

protected:
    QPaintEngine* paintEngine() const override { return nullptr; }
    void showEvent(QShowEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void initViewer();
    void displayPendingShape();
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

    TopoDS_Shape m_pendingShape;
    TopoDS_Shape m_currentShape;
    bool m_shaded = true;
    bool m_raytracing = false;
    bool m_viewerInitialized = false;

    QString m_materialPreset = QStringLiteral("steel");

    int m_reflectionBounces = 6;
    int m_refractionBounces = 4;
    double m_shadowSoftness = 0.5;
    bool m_shadowsEnabled = true;
    bool m_reflectionsEnabled = true;
    bool m_refractionsEnabled = true;
    double m_envIntensity = 0.3;

    Qt::MouseButtons m_mouseButtons = Qt::NoButton;
    QPoint m_lastPos;
};

} // namespace ExpressDesigner