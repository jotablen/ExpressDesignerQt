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

#include <QShowEvent>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QTimer>
#include <QDebug>
#include <utils/Logger.h>

namespace ExpressDesigner {

CADPreviewWidget::CADPreviewWidget(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_NativeWindow, true);
    setAttribute(Qt::WA_PaintOnScreen, true);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setMouseTracking(true);
    setMinimumSize(400, 300);
}

CADPreviewWidget::~CADPreviewWidget() = default;

// ============================================================================
// Initialization
// ============================================================================
void CADPreviewWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    if (!m_viewerInitialized) {
        initViewer();
        if (m_viewerInitialized) {
            displayPendingShape();
            // Defer MustBeResized to after Qt layout completes
            QTimer::singleShot(0, this, [this]() {
                if (!m_view.IsNull()) {
                    m_view->MustBeResized();
                    fitAll();
                    update();
                }
            });
        }
    }
}

void CADPreviewWidget::initViewer()
{
    if (m_viewerInitialized) return;

    LOG_INFO("CADPreview", "initViewer: starting...");
    qDebug() << "[CADPreview] initViewer: width=" << width() << "height=" << height();

    try {
        m_displayConnection = new Aspect_DisplayConnection();
        Handle(OpenGl_GraphicDriver) graphicDriver =
            new OpenGl_GraphicDriver(m_displayConnection);
        graphicDriver->ChangeOptions().buffersNoSwap = Standard_False;
        graphicDriver->ChangeOptions().useSystemBuffer = Standard_True;

        if (!graphicDriver->InitContext()) {
            LOG_WARN("CADPreview", "initViewer: InitContext FAILED");
            return;
        }
        LOG_INFO("CADPreview", "initViewer: InitContext OK");

        m_viewer = new V3d_Viewer(graphicDriver);
        m_viewer->SetDefaultViewSize(1000.0);
        m_viewer->SetDefaultViewProj(V3d_XposYnegZpos);
        m_viewer->SetComputedMode(Standard_False);
        m_viewer->SetDefaultLights();
        m_viewer->SetLightOn();

        m_context = new AIS_InteractiveContext(m_viewer);
        m_context->SetDisplayMode(AIS_Shaded, Standard_True);
        m_context->UpdateCurrentViewer();

        m_view = m_viewer->CreateView();
#ifdef _WIN32
        WId wid = this->winId();
        HWND hwnd = reinterpret_cast<HWND>(wid);
        Handle(WNT_Window) winHandle = new WNT_Window(hwnd);
        m_view->SetWindow(winHandle);
#else
        LOG_WARN("CADPreview", "initViewer: NOT on Windows");
#endif
        m_view->SetBackgroundColor(Quantity_NOC_ALICEBLUE);
        m_view->MustBeResized();

        setupDefaultLights();
        setupProceduralHDRI(QColor(135, 206, 235), QColor(220, 220, 220));

        Graphic3d_RenderingParams& rp = m_view->ChangeRenderingParams();
        rp.IsAntialiasingEnabled = Standard_True;
        rp.NbMsaaSamples = 8;
        rp.ToneMappingMethod = Graphic3d_ToneMappingMethod_Filmic;
        rp.AdaptiveScreenSampling = Standard_False;  // requires OpenGL 4.4

        m_viewerInitialized = true;
        LOG_INFO("CADPreview", "initViewer: SUCCESS");
    } catch (const std::exception& e) {
        LOG_WARN("CADPreview", QString("initViewer: EXCEPTION: %1").arg(e.what()));
        m_viewerInitialized = false;
    } catch (...) {
        LOG_WARN("CADPreview", "initViewer: UNKNOWN EXCEPTION");
        m_viewerInitialized = false;
    }
}

void CADPreviewWidget::displayPendingShape()
{
    if (!m_viewerInitialized) return;

    if (m_pendingShape.IsNull()) {
        m_context->RemoveAll(Standard_False);
        m_currentShape.Nullify();
        m_aisShape.Nullify();
        m_context->UpdateCurrentViewer();
        return;
    }

    m_currentShape = m_pendingShape;
    m_pendingShape.Nullify();
    m_aisShape.Nullify();

    m_context->RemoveAll(Standard_False);

    m_aisShape = new AIS_Shape(m_currentShape);
    m_context->SetDisplayMode(m_aisShape, m_shaded ? AIS_Shaded : AIS_WireFrame, Standard_False);
    updateMaterial();

    m_context->Display(m_aisShape, Standard_False);
    m_context->UpdateCurrentViewer();

    fitAll();
}

// ============================================================================
// Paint / Resize
// ============================================================================
void CADPreviewWidget::paintEvent(QPaintEvent*)
{
    if (!m_viewerInitialized && isVisible()) {
        initViewer();
        if (m_viewerInitialized) {
            // Ensure viewport matches widget size after first init
            m_view->MustBeResized();
            displayPendingShape();
        }
    }

    if (m_viewerInitialized && !m_view.IsNull())
        m_view->Redraw();
}

void CADPreviewWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (m_viewerInitialized && !m_view.IsNull()) {
        m_view->MustBeResized();
        fitAll();
    }
}

// ============================================================================
// Private helpers
// ============================================================================
void CADPreviewWidget::setupDefaultLights()
{
    if (m_viewer.IsNull()) return;
    try {
        m_viewer->SetLightOff();

        Handle(V3d_DirectionalLight) keyLight = new V3d_DirectionalLight(
            gp_Dir(1.0, -2.0, 3.0),
            Quantity_Color(0.9f, 0.85f, 0.75f, Quantity_TOC_sRGB));
        keyLight->SetHeadlight(Standard_False);
        keyLight->SetIntensity(1.2f);
        m_viewer->SetLightOn(keyLight);

        Handle(V3d_DirectionalLight) fillLight = new V3d_DirectionalLight(
            gp_Dir(-1.0, 1.5, 1.0),
            Quantity_Color(0.6f, 0.7f, 0.9f, Quantity_TOC_sRGB));
        fillLight->SetHeadlight(Standard_False);
        fillLight->SetIntensity(0.6f);
        m_viewer->SetLightOn(fillLight);

        Handle(V3d_DirectionalLight) rimLight = new V3d_DirectionalLight(
            gp_Dir(0.0, 0.5, -3.0),
            Quantity_Color(0.95f, 0.95f, 1.0f, Quantity_TOC_sRGB));
        rimLight->SetHeadlight(Standard_False);
        rimLight->SetIntensity(0.4f);
        m_viewer->SetLightOn(rimLight);
    } catch (const std::exception& e) {
        LOG_WARN("CADPreview", QString("setupDefaultLights threw: %1").arg(e.what()));
    } catch (...) {
        LOG_WARN("CADPreview", "setupDefaultLights threw UNKNOWN");
    }
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
        mat = Graphic3d_MaterialAspect(Graphic3d_NOM_COPPER);
    } else if (m_materialPreset == QStringLiteral("mirror")) {
        mat = Graphic3d_MaterialAspect(Graphic3d_NOM_CHROME);
    } else if (m_materialPreset == QStringLiteral("glass")) {
        mat = Graphic3d_MaterialAspect(Graphic3d_NOM_GLASS);
        mat.SetTransparency(0.2f);
        mat.SetRefractionIndex(1.5f);
    } else if (m_materialPreset == QStringLiteral("plastic")) {
        mat = Graphic3d_MaterialAspect(Graphic3d_NOM_PLASTIC);
    }

    m_aisShape->SetMaterial(mat);
    m_context->UpdateCurrentViewer();
}

void CADPreviewWidget::applyRaytracingParams()
{
    if (m_view.IsNull()) return;

    Graphic3d_RenderingParams& rp = m_view->ChangeRenderingParams();
    rp.Method = m_raytracing ? Graphic3d_RM_RAYTRACING : Graphic3d_RM_RASTERIZATION;
    rp.IsAntialiasingEnabled = Standard_True;
    rp.NbMsaaSamples = m_raytracing ? 8 : 4;

    if (m_raytracing) {
        rp.IsReflectionEnabled = m_reflectionsEnabled;
        rp.IsShadowEnabled = m_shadowsEnabled;
        rp.ToneMappingMethod = Graphic3d_ToneMappingMethod_Filmic;
        rp.AdaptiveScreenSampling = Standard_False;
        rp.RaytracingDepth = qMax(m_reflectionBounces, m_refractionBounces) + 2;
    }

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
    m_pendingShape = shape;

    if (!m_viewerInitialized && isVisible()) {
        initViewer();
    }

    if (!m_viewerInitialized) {
        return;
    }

    displayPendingShape();
    update();
}

void CADPreviewWidget::clearShape()
{
    m_pendingShape.Nullify();
    m_currentShape.Nullify();
    m_aisShape.Nullify();
    if (m_viewerInitialized && !m_context.IsNull()) {
        m_context->RemoveAll(Standard_True);
    }
}

void CADPreviewWidget::setShadingMode(bool shaded)
{
    m_shaded = shaded;
    if (m_viewerInitialized && !m_aisShape.IsNull()) {
        m_context->SetDisplayMode(m_aisShape, shaded ? AIS_Shaded : AIS_WireFrame, Standard_True);
        m_context->UpdateCurrentViewer();
    }
}

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
        m_view->Update();
    }
}

void CADPreviewWidget::setupProceduralHDRI(const QColor& skyTop, const QColor& skyBottom)
{
    if (m_view.IsNull()) return;
    try {
        m_view->SetBackgroundColor(Quantity_TOC_sRGB,
                                   skyTop.redF(), skyTop.greenF(), skyTop.blueF());
    } catch (const std::exception& e) {
        LOG_WARN("CADPreview", QString("setupProceduralHDRI threw: %1").arg(e.what()));
    } catch (...) {
        LOG_WARN("CADPreview", "setupProceduralHDRI threw UNKNOWN");
    }
}

bool CADPreviewWidget::saveSnapshot(const QString& filePath, int width, int height)
{
    if (m_view.IsNull()) return false;

    try {
        Image_PixMap image;
        m_view->ToPixMap(image, width, height);
        Handle(Image_AlienPixMap) alienImage = new Image_AlienPixMap();
        if (!alienImage->InitCopy(image)) return false;
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
        // Scale delta to reduce rotation sensitivity
        m_view->Rotate(delta.x() * 0.15, delta.y() * 0.15, 0.0);
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

void CADPreviewWidget::keyPressEvent(QKeyEvent* event)
{
    if (!m_viewerInitialized || m_view.IsNull()) return;

    switch (event->key()) {
    case Qt::Key_A:    // Fit-All
    case Qt::Key_Z:    // Zoom-All (same)
        fitAll();
        break;
    case Qt::Key_R:    // Reset view
        m_view->Reset();
        fitAll();
        break;
    case Qt::Key_W:    // Wireframe
        setShadingMode(false);
        break;
    case Qt::Key_S:    // Shaded
        setShadingMode(true);
        break;
    case Qt::Key_Plus:
    case Qt::Key_Equal:
        m_view->SetZoom(1.1, Standard_True);
        break;
    case Qt::Key_Minus:
        m_view->SetZoom(0.9, Standard_True);
        break;
    default:
        break;
    }
}

void CADPreviewWidget::wheelEvent(QWheelEvent* event)
{
    if (!m_viewerInitialized || m_view.IsNull()) return;
    double factor = (event->angleDelta().y() > 0) ? 1.1 : 0.9;
    m_view->SetZoom(factor, Standard_True);
}

} // namespace ExpressDesigner