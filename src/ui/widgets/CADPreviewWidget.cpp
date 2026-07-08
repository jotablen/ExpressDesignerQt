#include "CADPreviewWidget.h"

#include <OpenGl_GraphicDriver.hxx>
#include <AIS_Shape.hxx>
#include <AIS_DisplayMode.hxx>
#include <Quantity_Color.hxx>
#include <V3d_TypeOfOrientation.hxx>
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
        // Create the graphic driver with display connection
        m_displayConnection = new Aspect_DisplayConnection();
        Handle(OpenGl_GraphicDriver) graphicDriver =
            new OpenGl_GraphicDriver(m_displayConnection);
        graphicDriver->ChangeOptions().buffersNoSwap = Standard_True;
        graphicDriver->ChangeOptions().useSystemBuffer = Standard_False;
        if (!graphicDriver->Initialize()) {
            return;
        }

        // Create the 3D viewer
        m_viewer = new V3d_Viewer(graphicDriver);
        m_viewer->SetDefaultViewSize(1000.0);
        m_viewer->SetDefaultViewProj(V3d_XposYnegZpos);
        m_viewer->SetComputedMode(Standard_False);
        m_viewer->SetDefaultLights();
        m_viewer->SetLightOn();

        // Create the interactive context
        m_context = new AIS_InteractiveContext(m_viewer);
        m_context->SetDisplayMode(AIS_Shaded, Standard_True);
        m_context->SetHilightMode(AIS_Shaded);
        m_context->UpdateCurrentViewer();

        // Create the view with native window
        m_view = m_viewer->CreateView();

#ifdef _WIN32
        // Use native Windows HWND from QOpenGLWidget
        HWND hwnd = reinterpret_cast<HWND>(this->winId());
        Handle(WNT_Window) winHandle = new WNT_Window(hwnd);
        m_view->SetWindow(winHandle);
#endif

        m_view->SetBackgroundColor(Quantity_NOC_ALICEBLUE);
        m_view->MustBeResized();

        m_viewerInitialized = true;
    } catch (...) {
        // Failed to initialize viewer — proceed without OCCT visualization
        m_viewerInitialized = false;
    }
}

void CADPreviewWidget::resizeGL(int w, int h)
{
    if (m_viewerInitialized && !m_view.IsNull()) {
        m_view->MustBeResized();
        fitAll();
    }
}

void CADPreviewWidget::paintGL()
{
    if (m_viewerInitialized && !m_view.IsNull()) {
        m_view->Redraw();
    }
}

// ============================================================================
// Private helpers
// ============================================================================
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

    // Remove all displayed objects
    m_context->RemoveAll(Standard_False);

    if (shape.IsNull()) {
        m_context->UpdateCurrentViewer();
        return;
    }

    // Create new AIS shape with default material
    m_aisShape = new AIS_Shape(shape);
    m_context->SetDisplayMode(m_aisShape, m_shaded ? AIS_Shaded : AIS_WireFrame, Standard_False);

    // Display and update
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