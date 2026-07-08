#include "CADPreviewWidget.h"

#include <OpenGl_GraphicDriver.hxx>
#include <AIS_Shape.hxx>
#include <AIS_DisplayMode.hxx>
#include <Graphic3d_ShaderProgram.hxx>
#include <Graphic3d_TextureRoot.hxx>
#include <Graphic3d_GraphicDriver.hxx>
#include <Quantity_Color.hxx>
#include <Quantity_ColorRGBA.hxx>
#include <V3d_TypeOfOrientation.hxx>
#include <V3d_DirectionalLight.hxx>
#include <V3d_PositionalLight.hxx>
#include <Image_PixMap.hxx>
#include <Image_AlienPixMap.hxx>
#include <Aspect_NeutralWindow.hxx>
#include <Standard_Type.hxx>

#ifdef _WIN32
#include <WNT_Window.hxx>
#endif

namespace ExpressDesigner {

CADPreviewWidget::CADPreviewWidget(QWidget* parent)
    : QOpenGLWidget(parent)
{
    setMouseTracking(true);
    setMinimumSize(400, 300);
}

CADPreviewWidget::~CADPreviewWidget() = default;

// ============================================================================
// OpenGL Initialization
// ============================================================================
void CADPreviewWidget::initializeGL()
{
    makeCurrent();
    initViewer();
    doneCurrent();
}

void CADPreviewWidget::initViewer()
{
    if (m_viewerInitialized) return;

    try {
        m_displayConnection = new Aspect_DisplayConnection();
        Handle(OpenGl_GraphicDriver) graphicDriver =
            new OpenGl_GraphicDriver(m_displayConnection);
        graphicDriver->ChangeOptions().buffersNoSwap = Standard_True;
        graphicDriver->ChangeOptions().useSystemBuffer = Standard_False;
        if (!graphicDriver->Initialize()) return;

        m_viewer = new V3d_Viewer(graphicDriver);
        m_viewer->SetDefaultViewSize(1000.0);
        m_viewer->SetDefaultViewProj(V3d_XposYnegZpos);
        m_viewer->SetComputedMode(Standard_False);
        m_viewer->SetDefaultLights();
        m_viewer->SetLightOn();

        m_context = new AIS_InteractiveContext(m_viewer);
        m_context->SetDisplayMode(AIS_Shaded, Standard_True);
        m_context->SetHilightMode(AIS_Shaded);
        m_context->UpdateCurrentViewer();

        m_view = m_viewer->CreateView();
#ifdef _WIN32
        HWND hwnd = reinterpret_cast<HWND>(this->winId());
        Handle(WNT_Window) winHandle = new WNT_Window(hwnd);
        m_view->SetWindow(winHandle);
#endif
        m_view->SetBackgroundColor(Quantity_NOC_ALICEBLUE);
        m_view->MustBeResized();

        // Setup default lights
        setupDefaultLights();

        // Setup procedural HDRI
        setupProceduralHDRI(QColor(135, 206, 235), QColor(220, 220, 220));

        // High-quality defaults
        Graphic3d_RenderingParams& rp = m_view->ChangeRenderingParams();
        rp.IsAntialiasingEnabled = Standard_True;
        rp.NbMsaaSample = 8;
        rp.CollectedAspect = Graphic3d_Aspect_ToneMapping;

        m_viewerInitialized = true;
    } catch (...) {
        m_viewerInitialized = false;
    }
}

void CADPreviewWidget::resizeGL(int, int)
{
    if (m_viewerInitialized && !m_view.IsNull()) {
        m_view->MustBeResized();
        fitAll();
    }
}

void CADPreviewWidget::paintGL()
{
    if (m_viewerInitialized && !m_view.IsNull())
        m_view->Redraw();
}

// ============================================================================
// Private helpers
// ============================================================================
void CADPreviewWidget::setupDefaultLights()
{
    if (m_viewer.IsNull()) return;

    // Remove default lights
    m_viewer->SetLightOn(Standard_False);

    // Key light — warm directional from upper-right
    Handle(V3d_DirectionalLight) keyLight = new V3d_DirectionalLight(m_viewer,
        Quantity_Color(0.9f, 0.85f, 0.75f, Quantity_TOC_sRGB));
    keyLight->SetDirection(1.0, -2.0, 3.0);
    keyLight->SetHeadlight(Standard_False);
    keyLight->SetIntensity(1.2f);
    m_viewer->SetLightOn(keyLight);

    // Fill light — cool directional from lower-left
    Handle(V3d_DirectionalLight) fillLight = new V3d_DirectionalLight(m_viewer,
        Quantity_Color(0.6f, 0.7f, 0.9f, Quantity_TOC_sRGB));
    fillLight->SetDirection(-1.0, 1.5, 1.0);
    fillLight->SetHeadlight(Standard_False);
    fillLight->SetIntensity(0.6f);
    m_viewer->SetLightOn(fillLight);

    // Rim light — backlight from behind
    Handle(V3d_DirectionalLight) rimLight = new V3d_DirectionalLight(m_viewer,
        Quantity_Color(0.95f, 0.95f, 1.0f, Quantity_TOC_sRGB));
    rimLight->SetDirection(0.0, 0.5, -3.0);
    rimLight->SetHeadlight(Standard_False);
    rimLight->SetIntensity(0.4f);
    m_viewer->SetLightOn(rimLight);
}

void CADPreviewWidget::updateMaterial()
{
    if (m_aisShape.IsNull() || m_context.IsNull()) return;

    Graphic3d_MaterialAspect mat(Graphic3d_NOM_STEEL);

    if (m_materialPreset == QStringLiteral("steel")) {
        mat = Graphic3d_MaterialAspect(Graphic3d_NOM_STEEL);
    } else if (m_materialPreset == QStringLiteral("aluminium")) {
        mat = Graphic3d_MaterialAspect(Graphic3d_NOM_ALUMINIUM);
    } else if (m_materialPreset == QStringLiteral("gold")) {
        mat = Graphic3d_MaterialAspect(Graphic3d_NOM_GOLD);
    } else if (m_materialPreset == QStringLiteral("copper")) {
        // Copper-like: custom color with metallic reflection
        mat = Graphic3d_MaterialAspect(Graphic3d_NOM_COPPER);
    } else if (m_materialPreset == QStringLiteral("mirror")) {
        // High reflectivity, low diffuse
        mat = Graphic3d_MaterialAspect(Graphic3d_NOM_NEON_GRAY);
        mat.SetReflectionMode(Standard_True, Standard_True);
        mat.SetRefractionMode(Standard_False);
    } else if (m_materialPreset == QStringLiteral("glass")) {
        mat = Graphic3d_MaterialAspect(Graphic3d_NOM_GLASS);
        mat.SetTransparency(0.2f);
        mat.SetRefractionMode(Standard_True);
    } else if (m_materialPreset == QStringLiteral("plastic")) {
        mat = Graphic3d_MaterialAspect(Graphic3d_NOM_PLASTIC);
    } else {
        mat = Graphic3d_MaterialAspect(Graphic3d_NOM_STEEL);
    }

    m_aisShape->SetMaterial(mat);
    m_context->UpdateCurrentViewer();
}

void CADPreviewWidget::applyRaytracingParams()
{
    if (m_view.IsNull()) return;

    Graphic3d_RenderingParams& rp = m_view->ChangeRenderingParams();
    rp.Method = m_raytracing ? Graphic3d_RTM_RAYTRACING : Graphic3d_RTM_RASTERIZATION;
    rp.IsAntialiasingEnabled = m_raytracing;
    rp.NbMsaaSample = m_raytracing ? 8 : 4;

    if (m_raytracing) {
        rp.NbRayTracingReflections = m_reflectionsEnabled ? m_reflectionBounces : 0;
        rp.NbRayTracingRefractions = m_refractionsEnabled ? m_refractionBounces : 0;
        rp.IsShadowEnabled = m_shadowsEnabled;
        rp.NbRayTracingShadows = m_shadowSoftness > 0.01 ? 4 : 1;

        // Tone mapping (ACES-like filmic)
        rp.ToneMappingMethod = Graphic3d_ToneMapping_ACES;

        // Environment intensity (controls ambient reflections)
        rp.EnvMapIntensity = m_envIntensity;

        // Adaptive sampling
        rp.IsAdaptiveSamplingEnabled = Standard_True;
        rp.RaytracingDepth = qMax(m_reflectionBounces, m_refractionBounces) + 2;
    }

    // Anti-aliasing for both modes
    rp.IsAntialiasingEnabled = Standard_True;

    m_view->Update();
}

void CADPreviewWidget::fitAll()
{
    if (!m_view.IsNull() && !m_currentShape.IsNull()) {
        m_view->FitAll(0.01);
        m_view->ZFitAll();
    }
}

// ============================================================================
// Shape management
// ============================================================================
void CADPreviewWidget::setShape(const TopoDS_Shape& shape)
{
    if (!m_viewerInitialized) initViewer();
    if (!m_viewerInitialized) return;

    m_currentShape = shape;
    m_aisShape.Nullify();

    m_context->RemoveAll(Standard_False);

    if (shape.IsNull()) {
        m_context->UpdateCurrentViewer();
        return;
    }

    m_aisShape = new AIS_Shape(shape);
    m_context->SetDisplayMode(m_aisShape, m_shaded ? AIS_Shaded : AIS_WireFrame, Standard_False);
    updateMaterial();

    m_context->Display(m_aisShape, Standard_False);
    m_context->UpdateCurrentViewer();

    fitAll();
}

void CADPreviewWidget::clearShape()
{
    if (!m_viewerInitialized) return;
    m_currentShape.Nullify();
    m_aisShape.Nullify();
    m_context->RemoveAll(Standard_True);
}

void CADPreviewWidget::setShadingMode(bool shaded)
{
    m_shaded = shaded;
    if (m_viewerInitialized && !m_aisShape.IsNull()) {
        m_context->SetDisplayMode(m_aisShape, shaded ? AIS_Shaded : AIS_WireFrame, Standard_True);
        m_context->UpdateCurrentViewer();
    }
}

// ── Nivel 3: Raytracing control ──
void CADPreviewWidget::setRaytracingEnabled(bool enabled)
{
    m_raytracing = enabled;
    applyRaytracingParams();
}

void CADPreviewWidget::setMaterialPreset(const QString& preset)
{
    m_materialPreset = preset;
    updateMaterial();
}

void CADPreviewWidget::setReflectionBounces(int bounces)
{
    m_reflectionBounces = qBound(0, bounces, 10);
    applyRaytracingParams();
}

void CADPreviewWidget::setRefractionBounces(int bounces)
{
    m_refractionBounces = qBound(0, bounces, 10);
    applyRaytracingParams();
}

void CADPreviewWidget::setShadowSoftness(double softness)
{
    m_shadowSoftness = qBound(0.0, softness, 1.0);
    applyRaytracingParams();
}

void CADPreviewWidget::setShadowsEnabled(bool enabled)
{
    m_shadowsEnabled = enabled;
    applyRaytracingParams();
}

void CADPreviewWidget::setReflectionsEnabled(bool enabled)
{
    m_reflectionsEnabled = enabled;
    applyRaytracingParams();
}

void CADPreviewWidget::setRefractionsEnabled(bool enabled)
{
    m_refractionsEnabled = enabled;
    applyRaytracingParams();
}

void CADPreviewWidget::setEnvironmentIntensity(double intensity)
{
    m_envIntensity = qBound(0.0, intensity, 1.0);
    if (m_viewerInitialized && !m_view.IsNull()) {
        Graphic3d_RenderingParams& rp = m_view->ChangeRenderingParams();
        rp.EnvMapIntensity = m_envIntensity;
        m_view->Update();
    }
}

void CADPreviewWidget::setupProceduralHDRI(const QColor& skyTop, const QColor& skyBottom)
{
    // Generate a simple gradient texture for environment
    // OCCT can load a cubemap, but generating one procedurally is complex.
    // Instead, we set the background to a gradient and let raytracing use
    // the background color as environment reflection source.

    if (m_view.IsNull()) return;

    // Set gradient background for reflection/refraction environments
    m_view->SetBackgroundColor(Quantity_TOC_sRGB,
                               skyTop.redF(), skyTop.greenF(), skyTop.blueF());
    // Note: For true cubemap HDRI, we'd load an image via Graphic3d_CubeMap.
    // This gradient approach provides basic environment lighting.
}

bool CADPreviewWidget::saveSnapshot(const QString& filePath, int width, int height)
{
    if (m_view.IsNull()) return false;

    try {
        Image_PixMap image;
        m_view->ToPixMap(image, width, height);

        // Save to file using OCCT's Image_AlienPixMap
        Handle(Image_AlienPixMap) alienImage = new Image_AlienPixMap();
        if (!alienImage->InitWrapper(image)) return false;

        return alienImage->Save(filePath.toUtf8().constData());
    } catch (...) {
        return false;
    }
}

Graphic3d_RenderingParams& CADPreviewWidget::renderingParams()
{
    static Graphic3d_RenderingParams dummy;
    if (!m_view.IsNull())
        return m_view->ChangeRenderingParams();
    return dummy;
}

// ============================================================================
// Mouse interaction
// ============================================================================
void CADPreviewWidget::mousePressEvent(QMouseEvent* event)
{
    m_mouseButtons = event->buttons();
    m_lastPos = event->pos();
    setFocus();
}

void CADPreviewWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_viewerInitialized || m_view.IsNull()) return;

    QPoint delta = event->pos() - m_lastPos;
    m_lastPos = event->pos();

    if (m_mouseButtons & Qt::LeftButton) {
        m_view->Rotate(delta.x(), delta.y(), 0.0);
    } else if (m_mouseButtons & Qt::MiddleButton) {
        m_view->Pan(delta.x(), delta.y());
    } else if (m_mouseButtons & Qt::RightButton) {
        m_view->SetZoom(delta.y() > 0 ? 1.1 : 0.9, Standard_True);
    }
}

void CADPreviewWidget::mouseReleaseEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    m_mouseButtons = Qt::NoButton;
}

void CADPreviewWidget::wheelEvent(QWheelEvent* event)
{
    if (!m_viewerInitialized || m_view.IsNull()) return;
    double factor = (event->angleDelta().y() > 0) ? 1.1 : 0.9;
    m_view->SetZoom(factor, Standard_True);
}

} // namespace ExpressDesigner