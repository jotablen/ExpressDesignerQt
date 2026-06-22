#include "MainWindow.h"
#include <core/ObjectFactory.h>
#include <ui/dialogs/InsertObjectDialog.h>
#include <ui/dialogs/CalcOvalDialog.h>
#include <ui/dialogs/PropagateWFDialog.h>
#include <ui/dialogs/OffsetWFDialog.h>
#include <ui/dialogs/RotateObjectDialog.h>
#include <ui/dialogs/TranslateObjectDialog.h>
#include <ui/dialogs/DependencyGraphDialog.h>
#include <ui/dialogs/CopyObjectDialog.h>
#include <core/CarthesianOvalOperation.h>
#include <core/PropagateWFOperation.h>
#include <core/CurveObject.h>
#include <ui/widgets/PropertiesWidget.h>
#include <ui/dialogs/AboutDialog.h>
#include <ui/dialogs/ExportAllRhinoDialog.h>
#include <ui/dialogs/ImportObjectDialog.h>
#include <ui/dialogs/ProjectHistoryDialog.h>
#include <ui/dialogs/PreferencesDialog.h>
#include <ui/widgets/ChartWidget.h>
#include <ui/widgets/PropertiesWidget.h>
#include <io/ProjectSerializer.h>
#include <io/TXTExporter.h>
#include <io/TXTImporter.h>
#include <io/RhinoExporter.h>

#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QSettings>
#include <QLabel>
#include <QMenu>
#include <QAction>
#include <QSplitter>
#include <QVBoxLayout>
#include <QApplication>
#include <QMouseEvent>
#include <QWheelEvent>
#ifdef HAS_QT_CHARTS
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QValueAxis>
#endif

namespace ExpressDesigner {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_history(new HistoryManager)
    , m_depGraph(new DependencyGraph(this))
    , m_cmdHistory(new CommandHistory(this))
{
    setWindowTitle(QStringLiteral("ExpressDesigner"));
    resize(1280, 800);

    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    setupCentralWidget();
    setupConnections();

    onNewProject();

    QSettings settings;
    restoreGeometry(settings.value(QStringLiteral("MainWindow/geometry")).toByteArray());
    restoreState(settings.value(QStringLiteral("MainWindow/state")).toByteArray());
}

MainWindow::~MainWindow() = default;

void MainWindow::setProject(Project* project)
{
    if (m_currentProject && m_currentProject != project)
        delete m_currentProject;
    m_currentProject = project;

    if (m_treeModel)
        m_treeModel->setProject(project);
    if (m_propertiesWidget)
        m_propertiesWidget->setProject(project);

    m_selectedObject = nullptr;
    m_selectedOperation = nullptr;
    updateDeleteActionState();

    // Rebuild dependency graph
    if (m_depGraph && m_currentProject)
        m_depGraph->rebuildFromProject(m_currentProject);

    // Clear command history for new project
    if (m_cmdHistory)
        m_cmdHistory->clear();

    updateUndoRedoActions();
    refreshChart();
    updateStatusBar();
    setModified(false);
}

void MainWindow::setupMenuBar()
{
    // File menu
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(tr("&New Project"), QKeySequence::New, this, &MainWindow::onNewProject);
    fileMenu->addAction(tr("&Open Project..."), QKeySequence::Open, this, &MainWindow::onOpenProject);
    fileMenu->addAction(tr("&Save Project"), QKeySequence::Save, this, &MainWindow::onSaveProject);
    fileMenu->addAction(tr("Save Project &As..."), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S), this, &MainWindow::onSaveProjectAs);
    fileMenu->addAction(tr("&Close Project"), this, &MainWindow::onCloseProject);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("&Import Object..."), this, &MainWindow::onImportObject);
    fileMenu->addAction(tr("&Export Object..."), this, &MainWindow::onExportObject);
    fileMenu->addAction(tr("Export All to &Rhino..."), this, &MainWindow::onExportAllRhino);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("E&xit"), QKeySequence::Quit, this, &QWidget::close);

    // Edit menu
    QMenu* editMenu = menuBar()->addMenu(tr("&Edit"));
    m_undoAction = editMenu->addAction(tr("&Undo"), QKeySequence::Undo, this, &MainWindow::onUndo);
    m_redoAction = editMenu->addAction(tr("&Redo"), QKeySequence::Redo, this, &MainWindow::onRedo);
    editMenu->addSeparator();
    editMenu->addAction(tr("&Preferences..."), this, &MainWindow::onPreferences);
    updateUndoRedoActions();

    // Tasks menu
    QMenu* tasksMenu = menuBar()->addMenu(tr("&Tasks"));
    tasksMenu->addAction(tr("&Calculate Oval..."), this, &MainWindow::onCalculateOval);
    tasksMenu->addAction(tr("&Propagate WF..."), this, &MainWindow::onPropagateWF);
    tasksMenu->addAction(tr("&Offset WF..."), this, &MainWindow::onOffsetWF);
    tasksMenu->addSeparator();
    tasksMenu->addAction(tr("&Rotate Object..."), this, &MainWindow::onRotateObject);
    tasksMenu->addAction(tr("&Translate Object..."), this, &MainWindow::onTranslateObject);
    tasksMenu->addSeparator();
    tasksMenu->addAction(tr("&Copy Object..."), this, &MainWindow::onCopyObject);
    tasksMenu->addSeparator();
    tasksMenu->addAction(tr("&Recalculate All"), this, &MainWindow::onRecalculate);

    // View menu
    QMenu* viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(tr("Zoom &In"), QKeySequence::ZoomIn, this, &MainWindow::onZoomIn);
    viewMenu->addAction(tr("Zoom &Out"), QKeySequence::ZoomOut, this, &MainWindow::onZoomOut);
    viewMenu->addAction(tr("Zoom &All"), this, &MainWindow::onZoomAll);
    viewMenu->addSeparator();
    viewMenu->addAction(tr("&Dependency Graph..."), this, &MainWindow::onShowDependencyGraph);
    viewMenu->addSeparator();
    viewMenu->addAction(tr("Toggle &Control Points"), this, &MainWindow::onToggleControlPoints);
    viewMenu->addAction(tr("Toggle &Labels"), this, &MainWindow::onToggleLabels);
    viewMenu->addAction(tr("Toggle &Normals"), this, &MainWindow::onToggleNormals);

    // Help menu
    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(tr("&About"), this, &MainWindow::onAbout);
    helpMenu->addAction(tr("Project &History"), this, &MainWindow::onShowHistory);
}

void MainWindow::setupToolBar()
{
    QToolBar* toolbar = addToolBar(tr("Main"));
    toolbar->setMovable(false);

    toolbar->addAction(tr("Insert"), this, &MainWindow::onInsertObject);
    toolbar->addAction(tr("Calc Oval"), this, &MainWindow::onCalculateOval);
    toolbar->addAction(tr("Propagate"), this, &MainWindow::onPropagateWF);
    toolbar->addAction(tr("Offset"), this, &MainWindow::onOffsetWF);
    toolbar->addAction(tr("Rotate"), this, &MainWindow::onRotateObject);
    toolbar->addAction(tr("Translate"), this, &MainWindow::onTranslateObject);
    toolbar->addAction(tr("Copy"), this, &MainWindow::onCopyObject);
    toolbar->addSeparator();

    m_deleteAction = new QAction(tr("Delete"), this);
    connect(m_deleteAction, &QAction::triggered, this, &MainWindow::onDeleteObject);
    toolbar->addAction(m_deleteAction);
    updateDeleteActionState();
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::setupCentralWidget()
{
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);

    // Left panel: Object tree
    m_objectTree = new QTreeView(m_mainSplitter);
    m_treeModel = new ObjectTreeModel(this);
    m_objectTree->setModel(m_treeModel);
    m_objectTree->setHeaderHidden(false);
    m_objectTree->setMinimumWidth(200);

    // Right splitter (vertical)
    m_rightSplitter = new QSplitter(Qt::Vertical, m_mainSplitter);

    // Chart
#ifdef HAS_QT_CHARTS
    m_chartView = new QChartView(m_rightSplitter);
    m_chart = new QChart();
    m_chart->setTitle(tr("Project View"));
    m_chart->legend()->setAlignment(Qt::AlignRight);
    m_chartView->setChart(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    // Install event filter on both viewport (mouse) and chart view (resize)
    m_chartView->viewport()->installEventFilter(this);
    m_chartView->installEventFilter(this);
#else
    QWidget* chartPlaceholder = new QWidget(m_rightSplitter);
    chartPlaceholder->setStyleSheet(QStringLiteral("background-color: #f0f0f0;"));
    QLabel* phLabel = new QLabel(tr("Chart widget (Qt Charts not available)"), chartPlaceholder);
    phLabel->setAlignment(Qt::AlignCenter);
    QVBoxLayout* phLayout = new QVBoxLayout(chartPlaceholder);
    phLayout->addWidget(phLabel);
#endif

    // Properties with 8 tabs matching Ovals Designer pctrlObjProperties
    m_propertiesWidget = new PropertiesWidget(m_rightSplitter);
    m_propertiesWidget->setMinimumHeight(180);
    m_propertiesWidget->setCommandHistory(m_cmdHistory);

#ifdef HAS_QT_CHARTS
    m_rightSplitter->addWidget(m_chartView);
#else
    m_rightSplitter->addWidget(chartPlaceholder);
#endif
    m_rightSplitter->addWidget(m_propertiesWidget);
    m_rightSplitter->setStretchFactor(0, 1);
    m_rightSplitter->setStretchFactor(1, 0);
    m_rightSplitter->setCollapsible(1, false);
    m_rightSplitter->setSizes({600, 150});

    m_mainSplitter->addWidget(m_objectTree);
    m_mainSplitter->addWidget(m_rightSplitter);
    m_mainSplitter->setStretchFactor(0, 1);
    m_mainSplitter->setStretchFactor(1, 3);

    setCentralWidget(m_mainSplitter);
}

void MainWindow::setupConnections()
{
    connect(m_objectTree->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &MainWindow::onObjectSelected);

    connect(m_propertiesWidget, &PropertiesWidget::objectModified,
            this, [this](CustomObject* obj) {
                Q_UNUSED(obj);
                setModified(true);
                recalculateAll();
                refreshChart();
                updateStatusBar();
            });
    connect(m_propertiesWidget, &PropertiesWidget::projectModified,
            this, [this](Project*) {
                setModified(true);
                refreshChart();
                updateStatusBar();
            });

    connect(m_propertiesWidget, &PropertiesWidget::operationModified,
            this, [this](CustomOperation* op) {
                Q_UNUSED(op);
                setModified(true);
                recalculateAll();
                refreshChart();
                updateStatusBar();
            });

    connect(m_cmdHistory, &CommandHistory::stackChanged,
            this, &MainWindow::updateUndoRedoActions);

    connect(m_rightSplitter, &QSplitter::splitterMoved,
            this, [this](int, int) { maintainChartAspectRatio(); });
    connect(m_mainSplitter, &QSplitter::splitterMoved,
            this, [this](int, int) { maintainChartAspectRatio(); });

    m_objectTree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_objectTree, &QWidget::customContextMenuRequested,
            this, [this](const QPoint& pos) {
        QModelIndex idx = m_objectTree->indexAt(pos);
        CustomObject* obj = (idx.isValid()) ? m_treeModel->objectAt(idx) : nullptr;
        CustomOperation* op = (idx.isValid()) ? m_treeModel->operationAt(idx) : nullptr;
        QMenu menu(this);
        QAction* insertAct = menu.addAction(tr("&Insert object"));
        QAction* deleteAct = nullptr;
        QAction* hideAct = nullptr;
        QAction* copyAct = nullptr;
        if (obj) {
            menu.addSeparator();
            deleteAct = menu.addAction(tr("&Delete object"));
            hideAct = menu.addAction(tr("&Hide object"));
            copyAct = menu.addAction(tr("&Copy object..."));
        } else if (op) {
            menu.addSeparator();
            deleteAct = menu.addAction(tr("&Delete operation"));
        }
        menu.addSeparator();
        QAction* togglePtsAct = menu.addAction(tr("Toggle Ctrl Pts"));
        menu.addSeparator();
        QAction* zoomInAct = menu.addAction(tr("Zoom &In"));
        QAction* zoomOutAct = menu.addAction(tr("Zoom &Out"));
        QAction* zoomAllAct = menu.addAction(tr("Zoom &All"));
        QAction* chosen = menu.exec(m_objectTree->mapToGlobal(pos));
        if (chosen == insertAct) onInsertObject();
        else if (deleteAct && chosen == deleteAct) onDeleteObject();
        else if (hideAct && chosen == hideAct) {
            if (obj) { obj->setVisible(!obj->isVisible()); setModified(true); refreshChart(); }
        }
        else if (copyAct && chosen == copyAct) onCopyObject();
        else if (chosen == togglePtsAct) onToggleControlPoints();
        else if (chosen == zoomInAct) onZoomIn();
        else if (chosen == zoomOutAct) onZoomOut();
        else if (chosen == zoomAllAct) onZoomAll();
    });
}

// ============================================================================
// Event filter — dispatch to typed handler methods
// ============================================================================
bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
#ifdef HAS_QT_CHARTS
    // Chart view resize → maintain 1:1 aspect ratio
    if (m_chartView && watched == m_chartView) {
        if (event->type() == QEvent::Resize) {
            maintainChartAspectRatio();
            return false; // let Qt handle further
        }
    }

    // Chart viewport mouse/wheel events
    if (m_chartView && watched == m_chartView->viewport()) {
        switch (event->type()) {
        case QEvent::Wheel:
            return handleViewportWheel(static_cast<QWheelEvent*>(event));
        case QEvent::MouseButtonPress:
            return handleViewportMousePress(static_cast<QMouseEvent*>(event));
        case QEvent::MouseMove:
            return handleViewportMouseMove(static_cast<QMouseEvent*>(event));
        case QEvent::MouseButtonRelease:
            return handleViewportMouseRelease(static_cast<QMouseEvent*>(event));
        default:
            break;
        }
    }
#endif
    return QMainWindow::eventFilter(watched, event);
}

// ---------------------------------------------------------------------------
// handleViewportWheel — mouse-wheel zoom (1:1 centre-on-cursor)
// ---------------------------------------------------------------------------
bool MainWindow::handleViewportWheel(QWheelEvent* we)
{
#ifdef HAS_QT_CHARTS
    if (!m_chart) return true;
    QPoint vp = we->position().toPoint();
    QPointF cursorValue = m_chart->mapToValue(m_chartView->mapToScene(
        m_chartView->mapFromGlobal(m_chartView->viewport()->mapToGlobal(vp))));
    QValueAxis* ay = nullptr;
    for (auto* axis : m_chart->axes()) {
        if (axis->orientation() == Qt::Vertical) ay = qobject_cast<QValueAxis*>(axis);
    }
    if (ay) {
        double factor = (we->angleDelta().y() > 0) ? 0.9 : 1.1111;
        double yHalf = (ay->max() - ay->min()) * factor * 0.5;
        ay->setRange(cursorValue.y() - yHalf, cursorValue.y() + yHalf);
        maintainChartAspectRatio();
    }
#endif
    return true;
}

// ---------------------------------------------------------------------------
// handleViewportMousePress — Ctrl+click → drag CP  |  click → pan
// ---------------------------------------------------------------------------
bool MainWindow::handleViewportMousePress(QMouseEvent* me)
{
#ifdef HAS_QT_CHARTS
    if (me->button() != Qt::LeftButton) return false;

    // --- Ctrl+click: try to pick a control point of the selected data object ---
    if (me->modifiers() & Qt::ControlModifier) {
        m_isPanning = false;
        m_isDraggingCP = false;

        if (m_selectedObject && m_currentProject
            && m_currentProject->dataObjects().contains(m_selectedObject)) {
            QPointF chartPos = m_chart->mapToValue(me->pos());
            const auto& pts = m_selectedObject->controlPoints();
            int bestIdx = -1;
            double bestDist = 1e18;
            for (int i = 0; i < pts.size(); ++i) {
                double dx = pts[i].x() - chartPos.x();
                double dy = pts[i].y() - chartPos.y();
                double dist = qSqrt(dx * dx + dy * dy);
                if (dist < bestDist) { bestDist = dist; bestIdx = i; }
            }
            double snapDist = snapDistance(0.03);
            if (bestIdx >= 0 && bestDist <= snapDist) {
                m_isDraggingCP = true;
                m_dragCPIndex = bestIdx;
                m_dragCPOldPoints = pts;
                m_panLastPos = me->pos();
                return true;
            }
        }
        return true; // Ctrl pressed but nothing to drag → suppress pan/select
    }

    // --- Regular click: start pan ---
    m_isPanning = true;
    m_isDraggingCP = false;
    m_panLastPos = me->pos();
#endif
    return true;
}

// ---------------------------------------------------------------------------
// handleViewportMouseMove — drag CP (Ctrl) or pan chart
// ---------------------------------------------------------------------------
bool MainWindow::handleViewportMouseMove(QMouseEvent* me)
{
#ifdef HAS_QT_CHARTS
    if (m_isDraggingCP) {
        QPointF newPos = m_chart->mapToValue(me->pos());
        QVector<QPointF> pts = m_selectedObject->controlPoints();
        if (m_dragCPIndex >= 0 && m_dragCPIndex < pts.size()) {
            pts[m_dragCPIndex] = newPos;
            m_selectedObject->setControlPoints(pts);

            // Update the chart series in-place (no full rebuild) to avoid flicker
            const auto seriesList = m_chart->series();
            for (auto* ser : seriesList) {
                QVariant prop = ser->property("customObject");
                if (!prop.isValid()) continue;
                CustomObject* serObj = reinterpret_cast<CustomObject*>(prop.value<quintptr>());
                if (serObj != m_selectedObject) continue;

                if (auto* ls = qobject_cast<QLineSeries*>(ser)) {
                    if (m_dragCPIndex < ls->count())
                        ls->replace(m_dragCPIndex, newPos.x(), newPos.y());
                } else if (auto* spl = qobject_cast<QSplineSeries*>(ser)) {
                    if (m_dragCPIndex < spl->count())
                        spl->replace(m_dragCPIndex, newPos.x(), newPos.y());
                } else if (auto* ss = qobject_cast<QScatterSeries*>(ser)) {
                    if (m_dragCPIndex < ss->count())
                        ss->replace(m_dragCPIndex, newPos.x(), newPos.y());
                }
            }

            // Update PropertiesWidget grid if Curve tab is visible
            if (m_propertiesWidget)
                m_propertiesWidget->refreshGridFromObject(m_selectedObject);
        }
        return true;
    }

    if (m_isPanning) {
        QPoint vpNow = me->pos();
        QPoint vpPrev = m_panLastPos;
        QPoint cvNow  = m_chartView->mapFromGlobal(m_chartView->viewport()->mapToGlobal(vpNow));
        QPoint cvPrev = m_chartView->mapFromGlobal(m_chartView->viewport()->mapToGlobal(vpPrev));
        QPointF sceneNow  = m_chartView->mapToScene(cvNow);
        QPointF scenePrev = m_chartView->mapToScene(cvPrev);
        QPointF delta = sceneNow - scenePrev;
        m_chart->scroll(-delta.x(), delta.y());
        m_panLastPos = vpNow;
        return true;
    }
#endif
    return false;
}

// ---------------------------------------------------------------------------
// handleViewportMouseRelease — finish CP drag, or finish pan + select object
// ---------------------------------------------------------------------------
bool MainWindow::handleViewportMouseRelease(QMouseEvent* me)
{
#ifdef HAS_QT_CHARTS
    if (me->button() != Qt::LeftButton) return false;

    // --- Finish Ctrl+CP drag ---
    if (m_isDraggingCP && m_selectedObject) {
        QVector<QPointF> newPts = m_selectedObject->controlPoints();
        if (m_cmdHistory) {
            auto cmd = std::make_unique<ModifyControlPointsCommand>(
                m_selectedObject, m_dragCPOldPoints, newPts);
            m_cmdHistory->push(std::move(cmd), m_currentProject);
        }
        m_isDraggingCP = false;
        m_dragCPIndex = -1;
        setModified(true);
        recalculateAll();
        refreshChart();
        updateStatusBar();
        return true;
    }

    // --- Finish pan + click-select ---
    m_isPanning = false;
    m_isDraggingCP = false;

    QPointF chartPos = m_chart->mapToValue(me->pos());
    onChartClicked(chartPos);
    performChartClickSelect(chartPos);
#endif
    return true;
}

// ---------------------------------------------------------------------------
// snapDistance — compute a distance in chart data units as a percentage
// of the smaller visible axis range.  E.g. 0.05 = 5% snap tolerance.
// ---------------------------------------------------------------------------
double MainWindow::snapDistance(double percent) const
{
#ifdef HAS_QT_CHARTS
    if (!m_chart) return 10.0;
    QValueAxis* ax = nullptr;
    QValueAxis* ay = nullptr;
    for (auto* axis : m_chart->axes()) {
        if (axis->orientation() == Qt::Horizontal)
            ax = qobject_cast<QValueAxis*>(axis);
        else if (axis->orientation() == Qt::Vertical)
            ay = qobject_cast<QValueAxis*>(axis);
    }
    if (ax && ay) {
        double d = qMin(ax->max() - ax->min(), ay->max() - ay->min()) * percent;
        if (d < 0.1) d = 0.1;
        return d;
    }
#endif
    return 10.0;
}

// ---------------------------------------------------------------------------
void MainWindow::performChartClickSelect(const QPointF& chartPos)
{
#ifdef HAS_QT_CHARTS
    if (!m_chart || !m_treeModel) return;

    double snapDist = snapDistance(0.05);

    struct Candidate { CustomObject* obj = nullptr; double dist = 1e18; };
    Candidate nearest, secondNearest;

    for (auto* ser : m_chart->series()) {
        if (ser->name().isEmpty()) continue;
        QVariant prop = ser->property("customObject");
        if (!prop.isValid()) continue;
        CustomObject* obj = reinterpret_cast<CustomObject*>(prop.value<quintptr>());
        if (!obj) continue;

        auto checkPoint = [&](const QPointF& pt) {
            double dx = pt.x() - chartPos.x();
            double dy = pt.y() - chartPos.y();
            double dist = qSqrt(dx * dx + dy * dy);
            if (dist < nearest.dist) {
                secondNearest = nearest;
                nearest = {obj, dist};
            } else if (dist < secondNearest.dist && obj != nearest.obj) {
                secondNearest = {obj, dist};
            }
        };

        if (auto* ls = qobject_cast<QLineSeries*>(ser)) {
            for (int i = 0; i < ls->count(); ++i) checkPoint(ls->at(i));
        } else if (auto* spl = qobject_cast<QSplineSeries*>(ser)) {
            for (int i = 0; i < spl->count(); ++i) checkPoint(spl->at(i));
        } else if (auto* ss = qobject_cast<QScatterSeries*>(ser)) {
            for (int i = 0; i < ss->count(); ++i) checkPoint(ss->at(i));
        }
    }

    // If nearest is already selected, pick the second nearest (if in range)
    CustomObject* clickedObj = nullptr;
    if (nearest.obj) {
        if (nearest.obj == m_selectedObject && secondNearest.obj
            && secondNearest.dist <= snapDist) {
            clickedObj = secondNearest.obj;
        } else if (nearest.dist <= snapDist) {
            clickedObj = nearest.obj;
        }
    }

    if (clickedObj) {
        QModelIndex idx = m_treeModel->createIndexByObject(clickedObj);
        if (idx.isValid()) {
            m_objectTree->selectionModel()->setCurrentIndex(
                idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        }
    }
#endif
}

void MainWindow::onNewProject()
{
    auto* project = new Project(tr("New Project"), this);
    project->setName(tr("New Project"));
    m_currentFilePath.clear();
    setProject(project);
    m_history->addEntry(QStringLiteral("project_created"), project->name());
}

void MainWindow::onOpenProject()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Project"),
        QString(), tr("JSON Project (*.json);;All Files (*)"));
    if (filePath.isEmpty()) return;
    Project* loaded = ProjectSerializer::load(filePath);
    if (!loaded) { QMessageBox::critical(this, tr("Error"), tr("Failed to load project.")); return; }
    m_currentFilePath = filePath;
    setProject(loaded);
    m_history->addEntry(QStringLiteral("project_opened"), filePath);
}

void MainWindow::onSaveProject()
{
    if (m_currentFilePath.isEmpty()) { onSaveProjectAs(); return; }
    if (m_currentProject && ProjectSerializer::save(*m_currentProject, m_currentFilePath)) {
        setModified(false);
        statusBar()->showMessage(tr("Project saved."), 3000);
    }
}

void MainWindow::onSaveProjectAs()
{
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save Project As"),
        m_currentFilePath.isEmpty() ? QStringLiteral("project.json") : m_currentFilePath,
        tr("JSON Project (*.json);;All Files (*)"));
    if (filePath.isEmpty()) return;
    m_currentFilePath = filePath;
    onSaveProject();
}

void MainWindow::onCloseProject()
{
    if (m_isModified) {
        auto answer = QMessageBox::question(this, tr("Unsaved Changes"),
            tr("Save changes before closing?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (answer == QMessageBox::Cancel) return;
        if (answer == QMessageBox::Save) onSaveProject();
    }
    onNewProject();
}

void MainWindow::onInsertObject()
{
    InsertObjectDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted && m_currentProject) {
        CustomObject* obj = dlg.createdObject();
        if (obj) {
            m_currentProject->addDataObject(obj);
            m_history->recordObjectCreation(obj->name());
            setModified(true);
            refreshChart();
            updateStatusBar();
        }
    }
}

void MainWindow::onDeleteObject()
{
    QModelIndex idx = m_objectTree->currentIndex();
    if (!idx.isValid() || !m_currentProject) return;

    CustomObject* obj = m_treeModel->objectAt(idx);
    CustomOperation* op = m_treeModel->operationAt(idx);

    if (obj) {
        // --- Deleting an object ---
        // Check for dependent results
        if (m_depGraph) {
            m_depGraph->rebuildFromProject(m_currentProject);
            QSet<CustomObject*> dependents = m_depGraph->transitiveDependents(obj);
            if (!dependents.isEmpty()) {
                QStringList depNames;
                for (auto* dep : dependents)
                    depNames << dep->name();
                auto answer = QMessageBox::warning(this, tr("Object has dependents"),
                    tr("The object '%1' is used to calculate:\n%2\n\n"
                       "If you delete it, these results will no longer be recalculated.\n"
                       "Do you still want to delete it?")
                       .arg(obj->name(), depNames.join(QStringLiteral(", "))),
                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
                if (answer != QMessageBox::Yes) return;
            }
        }

        bool isResult = m_currentProject->resultObjects().contains(obj);
        CustomOperation* relatedOp = nullptr;

        // If deleting a result object, find the operation that produced it and remove it too
        if (isResult) {
            QString objName = obj->name();
            const auto& ops = m_currentProject->operations();
            for (auto* candidate : ops) {
                if (!candidate) continue;
                if (candidate->name() == objName || candidate->resultName() == objName) {
                    relatedOp = candidate;
                    break;
                }
                // If result was auto-renamed (e.g. "X_2"), original operation name could be "X"
                if (objName.startsWith(candidate->name() + QStringLiteral("_"))) {
                    relatedOp = candidate;
                    break;
                }
            }
        }

        auto cmd = std::make_unique<DeleteObjectCommand>(obj, isResult);
        m_cmdHistory->push(std::move(cmd), m_currentProject);
        if (m_depGraph)
            m_depGraph->removeObject(obj);

        // Also remove the associated operation if found
        if (relatedOp) {
            CustomOperation* takenOp = m_currentProject->takeOperation(relatedOp);
            delete takenOp; // clean up the orphan operation
        }

        m_history->recordObjectDeletion(obj->name());
        m_selectedObject = nullptr;
        m_selectedOperation = nullptr;
        updateDeleteActionState();
        updateUndoRedoActions();
        setModified(true);
        refreshChart();
        updateStatusBar();
    } else if (op) {
        // --- Deleting an operation ---
        // Ask if the user also wants to delete the result object
        CustomObject* resultObj = m_currentProject->findObject(op->resultName());
        QString resultName = op->resultName();

        bool deleteResult = false;
        if (resultObj) {
            auto answer = QMessageBox::question(this, tr("Delete Operation"),
                tr("Do you also want to delete the result object '%1'?").arg(resultName),
                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                QMessageBox::No);
            if (answer == QMessageBox::Cancel) return;
            deleteResult = (answer == QMessageBox::Yes);
        }

        // Remove operation from project
        CustomOperation* takenOp = m_currentProject->takeOperation(op);
        Q_UNUSED(takenOp);
        delete op; // delete the operation after taking it out of the project

        // Remove result object if requested
        if (deleteResult && resultObj) {
            m_currentProject->removeResultObject(resultObj);
            if (m_depGraph)
                m_depGraph->removeObject(resultObj);
            if (m_selectedObject == resultObj)
                m_selectedObject = nullptr;
        }

        if (m_selectedOperation == op)
            m_selectedOperation = nullptr;
        if (m_propertiesWidget)
            m_propertiesWidget->setOperation(nullptr);

        m_history->recordObjectDeletion(resultName);
        updateDeleteActionState();
        updateUndoRedoActions();
        setModified(true);
        refreshChart();
        updateStatusBar();
    }
}

void MainWindow::onEditObject() {}
void MainWindow::onImportObject()
{
    if (!m_currentProject) return;
    QString filePath = QFileDialog::getOpenFileName(this, tr("Import Object"),
        QString(), tr("TXT Files (*.txt);;All Files (*)"));
    if (filePath.isEmpty()) return;
    Q_UNUSED(filePath);
    refreshChart();
}

void MainWindow::onExportObject()
{
    QModelIndex idx = m_objectTree->currentIndex();
    if (!idx.isValid() || !m_currentProject) return;
    CustomObject* obj = m_treeModel->objectAt(idx);
    if (!obj) return;
    QString filePath = QFileDialog::getSaveFileName(this, tr("Export Object"),
        obj->name() + QStringLiteral(".txt"), tr("TXT Files (*.txt)"));
    if (filePath.isEmpty()) return;
    TXTExporter::exportToFile(obj, filePath);
}

void MainWindow::onExportAllRhino() {}

void MainWindow::onCalculateOval()
{
    if (!m_currentProject) return;
    CalcOvalDialog dlg(this);
    dlg.setProject(m_currentProject);
    if (dlg.exec() == QDialog::Accepted) {
        QString resultName = dlg.resultNameEdit()->text();
        // Check for name collision with existing objects
        CustomObject* existing = m_currentProject->findObject(resultName);
        if (existing) {
            QMessageBox nameDlg(this);
            nameDlg.setWindowTitle(tr("Name collision"));
            nameDlg.setText(tr("A result named '%1' already exists.\nWhat would you like to do?").arg(resultName));
            QPushButton* overwriteBtn = nameDlg.addButton(tr("Overwrite existing"), QMessageBox::AcceptRole);
            QPushButton* autoRenameBtn = nameDlg.addButton(tr("Auto-rename new"), QMessageBox::YesRole);
            QPushButton* cancelBtn = nameDlg.addButton(tr("Cancel"), QMessageBox::RejectRole);
            nameDlg.setDefaultButton(cancelBtn);
            nameDlg.exec();
            if (nameDlg.clickedButton() == overwriteBtn) {
                m_currentProject->removeResultObject(existing);
                m_currentProject->removeDataObject(existing);
            } else if (nameDlg.clickedButton() == autoRenameBtn) {
                // proceed — addResultObject will auto-rename
            } else {
                return; // Cancel
            }
        }
        auto* op = new CarthesianOvalOperation(resultName, this);
        op->setAmountOfPoints(dlg.amountEdit()->text().toInt());
        op->setParamName(CarthesianOvalOperation::PARAM_WF1, dlg.wfOriginCombo()->currentText());
        op->setParamName(CarthesianOvalOperation::PARAM_WF2, dlg.wfDestCombo()->currentText());
        op->setParamName(CarthesianOvalOperation::PARAM_REF_POINT, dlg.refPointCombo()->currentText());
        // push() internally calls execute() — operation runs and result is added
        auto execCmd = std::make_unique<ExecuteOperationCommand>(op);
        if (!m_cmdHistory->push(std::move(execCmd), m_currentProject)) {
            QMessageBox::warning(this, tr("Calculation failed"),
                tr("Cartesian Oval calculation failed.\nThe result has been removed."));
            return;
        }
        m_history->recordObjectCreation(op->name());
        setModified(true);
        updateUndoRedoActions();
        refreshChart();
        updateStatusBar();
    }
}

void MainWindow::onPropagateWF()
{
    if (!m_currentProject) return;
    PropagateWFDialog dlg(this);
    if (m_selectedObject && isWavefront(m_selectedObject->objectType()))
        dlg.setSelectedWF(m_selectedObject);
    dlg.setProject(m_currentProject);
    if (dlg.exec() == QDialog::Accepted) {
        QString resultName = dlg.resultNameEdit()->text();
        // Check for name collision with existing objects
        CustomObject* existing = m_currentProject->findObject(resultName);
        if (existing) {
            QMessageBox nameDlg(this);
            nameDlg.setWindowTitle(tr("Name collision"));
            nameDlg.setText(tr("A result named '%1' already exists.\nWhat would you like to do?").arg(resultName));
            QPushButton* overwriteBtn = nameDlg.addButton(tr("Overwrite existing"), QMessageBox::AcceptRole);
            QPushButton* autoRenameBtn = nameDlg.addButton(tr("Auto-rename new"), QMessageBox::YesRole);
            QPushButton* cancelBtn = nameDlg.addButton(tr("Cancel"), QMessageBox::RejectRole);
            nameDlg.setDefaultButton(cancelBtn);
            nameDlg.exec();
            if (nameDlg.clickedButton() == overwriteBtn) {
                m_currentProject->removeResultObject(existing);
                m_currentProject->removeDataObject(existing);
            } else if (nameDlg.clickedButton() == autoRenameBtn) {
                // proceed — addResultObject will auto-rename
            } else {
                return; // Cancel
            }
        }
        auto* op = new PropagateWFOperation(resultName, this);
        op->setAmountOfPoints(dlg.amountEdit()->text().toInt());
        op->setOffset(dlg.offsetEdit()->text().toDouble());
        op->setParamName(PropagateWFOperation::PARAM_WF, dlg.wfOrgCombo()->currentText());
        op->setParamName(PropagateWFOperation::PARAM_SURFACE, dlg.wfDestCombo()->currentText());
        op->setParamName(PropagateWFOperation::PARAM_IOR, dlg.indexDestEdit()->text());
        // push() internally calls execute() — operation runs and result is added
        auto execCmd = std::make_unique<ExecuteOperationCommand>(op);
        if (!m_cmdHistory->push(std::move(execCmd), m_currentProject)) {
            QMessageBox::warning(this, tr("Calculation failed"),
                tr("Propagate WF calculation failed.\nThe result has been removed."));
            return;
        }
        m_history->recordObjectCreation(op->name());
        setModified(true);
        updateUndoRedoActions();
        refreshChart();
        updateStatusBar();
    }
}

void MainWindow::onOffsetWF()
{
    if (!m_currentProject) return;
    OffsetWFDialog dlg(this);
    if (m_selectedObject && isWavefront(m_selectedObject->objectType()))
        dlg.setSelectedWF(m_selectedObject);
    dlg.setProject(m_currentProject);
    if (dlg.exec() == QDialog::Accepted) {
        CustomObject* srcWf = nullptr;
        QString wfName = dlg.wfCombo()->currentText();
        srcWf = m_currentProject->findObject(wfName);
        if (!srcWf) return;
        double offset = dlg.offsetEdit()->text().toDouble();
        QString resultName;
        int numPoints = srcWf->controlPointCount();
        if (numPoints < 2) { statusBar()->showMessage(tr("Offset WF requires at least 2 control points"), 5000); return; }
        auto normals = srcWf->computeNormals(numPoints);
        QVector<QPointF> offsetPts;
        offsetPts.reserve(numPoints);
        for (int i = 0; i < numPoints; ++i) {
            QPointF normalDir(1.0, 0.0);
            if (i < normals.size()) {
                QPointF vec = normals[i].second - normals[i].first;
                double len = qSqrt(vec.x() * vec.x() + vec.y() * vec.y());
                if (len > 1e-9) normalDir = vec / len;
            }
            QPointF pt = srcWf->controlPoints()[i];
            offsetPts.append(pt + normalDir * offset);
        }
        if (dlg.createNewResult()) {
            resultName = dlg.resultNameEdit()->text().trimmed();
            if (resultName.isEmpty()) resultName = wfName + QStringLiteral("_offset");
            auto* result = new CurveObject(resultName, true);
            result->setObjectType(withWavefront(withResult(ObjectType::Curve)));
            result->setRefractiveIndex(srcWf->refractiveIndex());
            result->setControlPoints(offsetPts);
            m_currentProject->addResultObject(result);
            m_history->recordObjectCreation(resultName);
        } else { srcWf->setControlPoints(offsetPts); }
        setModified(true);
        refreshChart();
        updateStatusBar();
    }
}

void MainWindow::onCopyObject()
{
    if (!m_currentProject) return;
    CopyObjectDialog dlg(this);
    dlg.setProject(m_currentProject, m_selectedObject ? m_selectedObject->name() : QString());
    if (dlg.exec() == QDialog::Accepted) {
        QString srcName = dlg.sourceName();
        QString newName = dlg.newName();
        if (srcName.isEmpty() || newName.isEmpty()) return;
        CustomObject* src = m_currentProject->findObject(srcName);
        if (!src) return;
        // Check for name collision
        CustomObject* existing = m_currentProject->findObject(newName);
        if (existing) {
            QMessageBox nameDlg(this);
            nameDlg.setWindowTitle(tr("Name collision"));
            nameDlg.setText(tr("An object named '%1' already exists.\nWhat would you like to do?").arg(newName));
            QPushButton* overwriteBtn = nameDlg.addButton(tr("Overwrite existing"), QMessageBox::AcceptRole);
            QPushButton* cancelBtn = nameDlg.addButton(tr("Cancel"), QMessageBox::RejectRole);
            nameDlg.setDefaultButton(cancelBtn);
            nameDlg.exec();
            if (nameDlg.clickedButton() == overwriteBtn) {
                m_currentProject->removeResultObject(existing);
                m_currentProject->removeDataObject(existing);
            } else {
                return; // Cancel
            }
        }
        // Manual copy: duplicate control points
        auto* copy = new CurveObject(newName, true);
        copy->setObjectType(src->objectType());
        copy->setControlPoints(src->controlPoints());
        copy->setRefractiveIndex(src->refractiveIndex());
        m_currentProject->addDataObject(copy);
        m_history->recordObjectCreation(newName);
        setModified(true);
        refreshChart();
        updateStatusBar();
    }
}

void MainWindow::onRecalculate() { recalculateAll(); refreshChart(); }

void MainWindow::onZoomIn()
{
#ifdef HAS_QT_CHARTS
    if (m_chart) {
        const auto axes = m_chart->axes();
        QValueAxis* ay = nullptr;
        for (auto* axis : axes) {
            if (axis->orientation() == Qt::Vertical) ay = qobject_cast<QValueAxis*>(axis);
        }
        if (ay) {
            double yCenter = (ay->max() + ay->min()) * 0.5;
            double yHalf = (ay->max() - ay->min()) * 0.5;
            double newHalf = yHalf * 0.5;  // halve visible range
            ay->setRange(yCenter - newHalf, yCenter + newHalf);
            maintainChartAspectRatio();
        }
    }
#endif
}

void MainWindow::onZoomOut()
{
#ifdef HAS_QT_CHARTS
    if (m_chart) {
        const auto axes = m_chart->axes();
        QValueAxis* ay = nullptr;
        for (auto* axis : axes) {
            if (axis->orientation() == Qt::Vertical) ay = qobject_cast<QValueAxis*>(axis);
        }
        if (ay) {
            double yCenter = (ay->max() + ay->min()) * 0.5;
            double yHalf = (ay->max() - ay->min()) * 0.5;
            double newHalf = yHalf * 2.0;  // double visible range
            ay->setRange(yCenter - newHalf, yCenter + newHalf);
            maintainChartAspectRatio();
        }
    }
#endif
}

void MainWindow::onZoomAll()
{
#ifdef HAS_QT_CHARTS
    refreshChart();
#endif
}

void MainWindow::onToggleControlPoints() { m_showControlPoints = !m_showControlPoints; refreshChart(); }
void MainWindow::onToggleLabels() { m_showLabels = !m_showLabels; refreshChart(); }
void MainWindow::onToggleNormals() { m_showNormals = !m_showNormals; refreshChart(); }
void MainWindow::onSetAspectRatio() {}

void MainWindow::maintainChartAspectRatio()
{
#ifdef HAS_QT_CHARTS
    if (!m_chart) return;
    const auto axes = m_chart->axes();
    QValueAxis* ax = nullptr;
    QValueAxis* ay = nullptr;
    for (auto* axis : axes) {
        if (axis->orientation() == Qt::Horizontal) ax = qobject_cast<QValueAxis*>(axis);
        else if (axis->orientation() == Qt::Vertical) ay = qobject_cast<QValueAxis*>(axis);
    }
    if (!ax || !ay) return;

    // ODs approach: adjust X axis based on Y range and chart pixel dimensions
    double yRange = ay->max() - ay->min();
    if (yRange < 1e-9) return;

    QRectF plotArea = m_chart->plotArea();
    double chartWidth = plotArea.width();
    double chartHeight = plotArea.height();
    if (chartHeight < 1.0) return;

    double xRange = chartWidth * yRange / chartHeight;
    double xCenter = (ax->max() + ax->min()) * 0.5;
    ax->setRange(xCenter - xRange * 0.5, xCenter + xRange * 0.5);
#endif
}

void MainWindow::onPreferences() { PreferencesDialog dlg(this); dlg.exec(); }

void MainWindow::updateDeleteActionState()
{
    bool canDelete = false;
    if (m_currentProject) {
        if (m_selectedObject) {
            const auto& dataObjs = m_currentProject->dataObjects();
            const auto& resultObjs = m_currentProject->resultObjects();
            if (dataObjs.contains(m_selectedObject) || resultObjs.contains(m_selectedObject))
                canDelete = true;
        }
        if (m_selectedOperation) {
            const auto& ops = m_currentProject->operations();
            if (ops.contains(m_selectedOperation))
                canDelete = true;
        }
    }
    if (m_deleteAction) m_deleteAction->setEnabled(canDelete);
}

void MainWindow::onObjectSelected(const QModelIndex& index)
{
    // Check if this is an operation node
    auto* op = m_treeModel->operationAt(index);
    if (op) {
        m_selectedObject = nullptr;
        m_selectedOperation = op;
        if (m_propertiesWidget) m_propertiesWidget->setOperation(op);
        updateDeleteActionState();
        updateStatusBar();
        return;
    }

    m_selectedObject = m_treeModel->objectAt(index);
    m_selectedOperation = nullptr;
    if (m_propertiesWidget) m_propertiesWidget->setObject(m_selectedObject);
    updateDeleteActionState();
    refreshChart();
    updateStatusBar();
}

void MainWindow::onChartClicked(const QPointF& point)
{
    statusBar()->showMessage(tr("Position: (%1, %2)").arg(point.x(), 0, 'f', 2).arg(point.y(), 0, 'f', 2));
}

void MainWindow::onAbout() { AboutDialog dlg(this); dlg.exec(); }
void MainWindow::onShowHistory() {}

void MainWindow::onShowDependencyGraph()
{
    if (!m_currentProject) return;
    DependencyGraphDialog dlg(this);
    dlg.setProject(m_currentProject, m_depGraph);
    dlg.exec();
}

void MainWindow::onRotateObject()
{
    if (!m_currentProject) return;
    RotateObjectDialog dlg(this);
    dlg.setProject(m_currentProject);
    if (m_selectedObject)
        dlg.setSelectedObject(m_selectedObject);
    if (dlg.exec() == QDialog::Accepted) {
        double degrees = dlg.degreesEdit()->text().toDouble();
        RotateObjectCommand::PivotMode pivot =
            static_cast<RotateObjectCommand::PivotMode>(dlg.pivotMode());
        const auto selected = dlg.objectList()->selectedItems();
        if (selected.isEmpty()) return;
        for (auto* item : selected) {
            CustomObject* obj = m_currentProject->findObject(item->text());
            if (!obj) continue;
            auto cmd = std::make_unique<RotateObjectCommand>(obj, degrees, pivot);
            m_cmdHistory->push(std::move(cmd), m_currentProject);
            m_history->recordObjectModification(obj->name(), QStringLiteral("rotate"),
                QString(), QString::number(degrees) + QStringLiteral("°"));
        }
        setModified(true);
        recalculateAll();
        refreshChart();
        updateStatusBar();
    }
}

void MainWindow::onTranslateObject()
{
    if (!m_currentProject) return;
    TranslateObjectDialog dlg(this);
    dlg.setProject(m_currentProject);
    if (m_selectedObject)
        dlg.setSelectedObject(m_selectedObject);
    if (dlg.exec() == QDialog::Accepted) {
        double dx = dlg.deltaXEdit()->text().toDouble();
        double dy = dlg.deltaYEdit()->text().toDouble();
        QPointF delta(dx, dy);
        const auto selected = dlg.objectList()->selectedItems();
        if (selected.isEmpty()) return;
        for (auto* item : selected) {
            CustomObject* obj = m_currentProject->findObject(item->text());
            if (!obj) continue;
            auto cmd = std::make_unique<TranslateObjectCommand>(obj, delta);
            m_cmdHistory->push(std::move(cmd), m_currentProject);
            m_history->recordObjectModification(obj->name(), QStringLiteral("translate"),
                QString(), QStringLiteral("Δ(%1,%2)").arg(dx).arg(dy));
        }
        setModified(true);
        recalculateAll();
        refreshChart();
        updateStatusBar();
    }
}

void MainWindow::onUndo()
{
    if (!m_cmdHistory || !m_cmdHistory->canUndo()) return;

    m_cmdHistory->undo(m_currentProject);

    // Undo may have deleted objects (e.g., result objects from operations);
    // clear the selection to avoid dangling pointer in refreshChart()
    m_selectedObject = nullptr;
    m_selectedOperation = nullptr;
    if (m_propertiesWidget)
        m_propertiesWidget->setObject(nullptr);

    recalculateAll();
    updateUndoRedoActions();
    refreshChart();
    updateStatusBar();
    setModified(true);
}

void MainWindow::onRedo()
{
    if (!m_cmdHistory || !m_cmdHistory->canRedo()) return;

    m_cmdHistory->redo(m_currentProject);

    // If the result object was re-created, the old selection pointer is stale
    m_selectedObject = nullptr;
    m_selectedOperation = nullptr;
    if (m_propertiesWidget)
        m_propertiesWidget->setObject(nullptr);

    recalculateAll();
    updateUndoRedoActions();
    refreshChart();
    updateStatusBar();
    setModified(true);
}

void MainWindow::updateUndoRedoActions()
{
    if (m_undoAction) {
        m_undoAction->setEnabled(m_cmdHistory ? m_cmdHistory->canUndo() : false);
        if (m_cmdHistory && m_cmdHistory->canUndo())
            m_undoAction->setText(m_cmdHistory->undoText());
        else
            m_undoAction->setText(tr("&Undo"));
    }
    if (m_redoAction) {
        m_redoAction->setEnabled(m_cmdHistory ? m_cmdHistory->canRedo() : false);
        if (m_cmdHistory && m_cmdHistory->canRedo())
            m_redoAction->setText(m_cmdHistory->redoText());
        else
            m_redoAction->setText(tr("&Redo"));
    }
}

void MainWindow::recalculateAll()
{
    if (!m_currentProject) return;

    // Block all project signals during recalculation
    const QSignalBlocker blocker(m_currentProject);

    // Remove all current result objects
    for (int i = m_currentProject->resultObjectCount() - 1; i >= 0; --i)
        m_currentProject->removeResultObject(i);

    // Re-execute every operation in insertion order
    const auto& ops = m_currentProject->operations();
    for (auto* op : ops) {
        if (!op) continue;
        op->execute(m_currentProject);
    }

    // Rebuild dependency graph ONCE for UI queries (dialog, delete warnings)
    if (m_depGraph)
        m_depGraph->rebuildFromProject(m_currentProject);
}

void MainWindow::refreshChart()
{
#ifdef HAS_QT_CHARTS
    if (!m_chart || !m_currentProject) return;
    m_chart->removeAllSeries();
    ChartWidget::populateChart(m_chart, m_currentProject, m_showControlPoints, m_showNormals, m_selectedObject);
    // Ensure 1:1 aspect ratio after chart population (deferred to allow layout)
    QMetaObject::invokeMethod(this, [this]() { maintainChartAspectRatio(); }, Qt::QueuedConnection);
#endif
}

void MainWindow::updateStatusBar()
{
    if (!m_currentProject) { statusBar()->showMessage(tr("No project open")); return; }
    int totalObjects = m_currentProject->dataObjectCount() + m_currentProject->resultObjectCount();
    statusBar()->showMessage(tr("Ready | Objects: %1").arg(totalObjects));
}

void MainWindow::setModified(bool modified)
{
    m_isModified = modified;
    setWindowTitle(modified ? QStringLiteral("ExpressDesigner *") : QStringLiteral("ExpressDesigner"));
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (m_isModified) {
        auto answer = QMessageBox::question(this, tr("Unsaved Changes"),
            tr("Save changes before exiting?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (answer == QMessageBox::Cancel) { event->ignore(); return; }
        if (answer == QMessageBox::Save) onSaveProject();
    }
    QSettings settings;
    settings.setValue(QStringLiteral("MainWindow/geometry"), saveGeometry());
    settings.setValue(QStringLiteral("MainWindow/state"), saveState());
    event->accept();
}

} // namespace ExpressDesigner