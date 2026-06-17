#include "MainWindow.h"
#include <core/ObjectFactory.h>
#include <ui/dialogs/InsertObjectDialog.h>
#include <ui/dialogs/CalcOvalDialog.h>
#include <ui/dialogs/PropagateWFDialog.h>
#include <ui/dialogs/OffsetWFDialog.h>
#include <ui/dialogs/RotateObjectDialog.h>
#include <ui/dialogs/TranslateObjectDialog.h>
#include <ui/dialogs/DependencyGraphDialog.h>
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
    m_propertiesWidget->setMinimumHeight(150);

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
                setModified(true);
                recalcDependents(obj);
                refreshChart();
                updateStatusBar();
            });
    connect(m_propertiesWidget, &PropertiesWidget::projectModified,
            this, [this](Project*) {
                setModified(true);
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
        QMenu menu(this);
        QAction* insertAct = menu.addAction(tr("&Insert object"));
        QAction* deleteAct = nullptr;
        QAction* hideAct = nullptr;
        if (obj) {
            menu.addSeparator();
            deleteAct = menu.addAction(tr("&Delete object"));
            hideAct = menu.addAction(tr("&Hide object"));
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
        else if (chosen == togglePtsAct) onToggleControlPoints();
        else if (chosen == zoomInAct) onZoomIn();
        else if (chosen == zoomOutAct) onZoomOut();
        else if (chosen == zoomAllAct) onZoomAll();
    });
}

// Event filter
bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
#ifdef HAS_QT_CHARTS
    if (m_chartView && watched == m_chartView) {
        if (event->type() == QEvent::Resize) {
            maintainChartAspectRatio();
            return false;
        }
    }

    if (m_chartView && watched == m_chartView->viewport()) {
        // --- Mouse wheel zoom (1:1) ---
        if (event->type() == QEvent::Wheel) {
            QWheelEvent* we = static_cast<QWheelEvent*>(event);
            if (m_chart) {
                // viewport-local → chart-view-local → scene coordinates
                QPoint vp = we->position().toPoint();
                QPointF cursorValue = m_chart->mapToValue(m_chartView->mapToScene(
                    m_chartView->mapFromGlobal(m_chartView->viewport()->mapToGlobal(vp))));
                const auto axes = m_chart->axes();
                QValueAxis* ay = nullptr;
                for (auto* axis : axes) {
                    if (axis->orientation() == Qt::Vertical) ay = qobject_cast<QValueAxis*>(axis);
                }
                if (ay) {
                    double factor = (we->angleDelta().y() > 0) ? 0.9 : 1.1111;
                    double yHalf = (ay->max() - ay->min()) * factor * 0.5;
                    ay->setRange(cursorValue.y() - yHalf, cursorValue.y() + yHalf);
                    maintainChartAspectRatio();
                }
            }
            return true;
        }

        // --- Mouse press → begin pan ---
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::LeftButton) {
                m_isPanning = true;
                m_panLastPos = me->pos();
                return true;
            }
        }

        // --- Mouse move → pan chart ---
        if (event->type() == QEvent::MouseMove && m_isPanning) {
            QMouseEvent* me = static_cast<QMouseEvent*>(event);
            if (m_chart) {
                QPoint vpNow = me->pos();
                QPoint vpPrev = m_panLastPos;
                QPoint cvNow  = m_chartView->mapFromGlobal(m_chartView->viewport()->mapToGlobal(vpNow));
                QPoint cvPrev = m_chartView->mapFromGlobal(m_chartView->viewport()->mapToGlobal(vpPrev));
                QPointF sceneNow  = m_chartView->mapToScene(cvNow);
                QPointF scenePrev = m_chartView->mapToScene(cvPrev);
                QPointF delta = sceneNow - scenePrev;
                m_chart->scroll(-delta.x(), delta.y());
                m_panLastPos = vpNow;
            }
            return true;
        }

        // --- Mouse release → end pan + click-select ---
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent* me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::LeftButton) {
                m_isPanning = false;

                QPoint vp = me->pos();
                QPoint cv = m_chartView->mapFromGlobal(m_chartView->viewport()->mapToGlobal(vp));
                QPointF chartPos = m_chartView->mapToScene(cv);
                onChartClicked(chartPos);

                double snapDist = 10.0;
                const auto axes = m_chart->axes();
                if (axes.size() >= 2) {
                    auto* ax = qobject_cast<QValueAxis*>(axes[0]);
                    auto* ay = qobject_cast<QValueAxis*>(axes[1]);
                    if (ax && ay) {
                        snapDist = qMin(ax->max() - ax->min(), ay->max() - ay->min()) * 0.05;
                        if (snapDist < 2.0) snapDist = 2.0;
                    }
                }

                CustomObject* clickedObj = nullptr;
                const auto seriesList = m_chart->series();
                double bestDist = snapDist;
                for (auto* ser : seriesList) {
                    if (ser->name().isEmpty()) continue;
                    QVariant prop = ser->property("customObject");
                    if (!prop.isValid()) continue;

                    QLineSeries* ls = qobject_cast<QLineSeries*>(ser);
                    QScatterSeries* ss = qobject_cast<QScatterSeries*>(ser);
                    if (ls) {
                        for (int i = 0; i < ls->count(); ++i) {
                            QPointF pt = ls->at(i);
                            double dx = pt.x() - chartPos.x();
                            double dy = pt.y() - chartPos.y();
                            double dist = qSqrt(dx * dx + dy * dy);
                            if (dist < bestDist) {
                                bestDist = dist;
                                clickedObj = reinterpret_cast<CustomObject*>(prop.value<quintptr>());
                            }
                        }
                    } else if (ss) {
                        for (int i = 0; i < ss->count(); ++i) {
                            QPointF pt = ss->at(i);
                            double dx = pt.x() - chartPos.x();
                            double dy = pt.y() - chartPos.y();
                            double dist = qSqrt(dx * dx + dy * dy);
                            if (dist < bestDist) {
                                bestDist = dist;
                                clickedObj = reinterpret_cast<CustomObject*>(prop.value<quintptr>());
                            }
                        }
                    }
                }

                if (clickedObj && m_treeModel) {
                    QModelIndex idx = m_treeModel->createIndexByObject(clickedObj);
                    if (idx.isValid()) {
                        m_objectTree->selectionModel()->setCurrentIndex(
                            idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
                    }
                }
            }
            return true;
        }
    }
#endif
    return QMainWindow::eventFilter(watched, event);
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
    if (!obj) return;

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
    auto cmd = std::make_unique<DeleteObjectCommand>(obj, isResult);
    m_cmdHistory->push(std::move(cmd), m_currentProject);
    if (m_depGraph)
        m_depGraph->removeObject(obj);
    m_history->recordObjectDeletion(obj->name());
    m_selectedObject = nullptr;
    updateDeleteActionState();
    updateUndoRedoActions();
    setModified(true);
    refreshChart();
    updateStatusBar();
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
        auto* op = new CarthesianOvalOperation(dlg.resultNameEdit()->text(), this);
        op->setAmountOfPoints(dlg.amountEdit()->text().toInt());
        op->setParamName(CarthesianOvalOperation::PARAM_WF1, dlg.wfOriginCombo()->currentText());
        op->setParamName(CarthesianOvalOperation::PARAM_WF2, dlg.wfDestCombo()->currentText());
        op->setParamName(CarthesianOvalOperation::PARAM_REF_POINT, dlg.refPointCombo()->currentText());
        // push() internally calls execute()
        auto execCmd = std::make_unique<ExecuteOperationCommand>(op);
        m_cmdHistory->push(std::move(execCmd), m_currentProject);
        m_currentProject->addOperation(op);
        m_history->recordObjectCreation(op->name());
        setModified(true);
        if (m_depGraph) m_depGraph->rebuildFromProject(m_currentProject);
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
        auto* op = new PropagateWFOperation(dlg.resultNameEdit()->text(), this);
        op->setAmountOfPoints(dlg.amountEdit()->text().toInt());
        op->setOffset(dlg.offsetEdit()->text().toDouble());
        op->setParamName(PropagateWFOperation::PARAM_WF, dlg.wfOrgCombo()->currentText());
        op->setParamName(PropagateWFOperation::PARAM_SURFACE, dlg.wfDestCombo()->currentText());
        op->setParamName(PropagateWFOperation::PARAM_IOR, dlg.indexDestEdit()->text());
        // push() internally calls execute()
        auto execCmd = std::make_unique<ExecuteOperationCommand>(op);
        m_cmdHistory->push(std::move(execCmd), m_currentProject);
        m_currentProject->addOperation(op);
        m_history->recordObjectCreation(op->name());
        setModified(true);
        if (m_depGraph) m_depGraph->rebuildFromProject(m_currentProject);
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

void MainWindow::onRecalculate() { refreshChart(); }

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
    if (m_selectedObject && m_currentProject) {
        const auto& dataObjs = m_currentProject->dataObjects();
        const auto& resultObjs = m_currentProject->resultObjects();
        if (dataObjs.contains(m_selectedObject) || resultObjs.contains(m_selectedObject))
            canDelete = true;
    }
    if (m_deleteAction) m_deleteAction->setEnabled(canDelete);
}

void MainWindow::onObjectSelected(const QModelIndex& index)
{
    m_selectedObject = m_treeModel->objectAt(index);
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
    if (m_selectedObject)
        dlg.setSelectedObject(m_selectedObject);
    dlg.setProject(m_currentProject);
    if (dlg.exec() == QDialog::Accepted) {
        CustomObject* obj = m_currentProject->findObject(dlg.objectCombo()->currentText());
        if (!obj) return;
        double degrees = dlg.degreesEdit()->text().toDouble();
        RotateObjectCommand::PivotMode pivot =
            static_cast<RotateObjectCommand::PivotMode>(dlg.pivotMode());
        auto cmd = std::make_unique<RotateObjectCommand>(obj, degrees, pivot);
        m_cmdHistory->push(std::move(cmd), m_currentProject);
        m_history->recordObjectModification(obj->name(), QStringLiteral("rotate"),
            QString(), QString::number(degrees) + QStringLiteral("°"));
        setModified(true);
        recalcDependents(obj);
        refreshChart();
        updateStatusBar();
    }
}

void MainWindow::onTranslateObject()
{
    if (!m_currentProject) return;
    TranslateObjectDialog dlg(this);
    if (m_selectedObject)
        dlg.setSelectedObject(m_selectedObject);
    dlg.setProject(m_currentProject);
    if (dlg.exec() == QDialog::Accepted) {
        CustomObject* obj = m_currentProject->findObject(dlg.objectCombo()->currentText());
        if (!obj) return;
        double dx = dlg.deltaXEdit()->text().toDouble();
        double dy = dlg.deltaYEdit()->text().toDouble();
        QPointF delta(dx, dy);
        auto cmd = std::make_unique<TranslateObjectCommand>(obj, delta);
        m_cmdHistory->push(std::move(cmd), m_currentProject);
        m_history->recordObjectModification(obj->name(), QStringLiteral("translate"),
            QString(), QStringLiteral("Δ(%1,%2)").arg(dx).arg(dy));
        setModified(true);
        recalcDependents(obj);
        refreshChart();
        updateStatusBar();
    }
}

void MainWindow::onUndo()
{
    if (!m_cmdHistory || !m_cmdHistory->canUndo()) return;

    m_cmdHistory->undo(m_currentProject);

    // Rebuild graph and recalculate dependents for the affected object
    if (m_depGraph) {
        m_depGraph->rebuildFromProject(m_currentProject);
        QString objName = m_cmdHistory->lastUndoneModifiedObjectName();
        if (!objName.isEmpty()) {
            CustomObject* modifiedObj = m_currentProject->findObject(objName);
            if (modifiedObj)
                recalcDependents(modifiedObj);
        }
    }

    updateUndoRedoActions();
    refreshChart();
    updateStatusBar();
    setModified(true);
}

void MainWindow::onRedo()
{
    if (!m_cmdHistory || !m_cmdHistory->canRedo()) return;

    m_cmdHistory->redo(m_currentProject);

    // Rebuild graph and recalculate dependents for the affected object
    if (m_depGraph) {
        m_depGraph->rebuildFromProject(m_currentProject);
        QString objName = m_cmdHistory->lastRedoneModifiedObjectName();
        if (!objName.isEmpty()) {
            CustomObject* modifiedObj = m_currentProject->findObject(objName);
            if (modifiedObj)
                recalcDependents(modifiedObj);
        }
    }

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

void MainWindow::recalcDependents(CustomObject* modifiedObj)
{
    if (!m_currentProject || !m_depGraph) return;
    if (m_recalcInProgress) return;
    m_recalcInProgress = true;

    // Rebuild dependency graph to stay in sync
    m_depGraph->rebuildFromProject(m_currentProject);

    if (!modifiedObj) { m_recalcInProgress = false; return; }

    // Transitive recalculation: keep recalculating until no more dependents
    QSet<CustomOperation*> processed;   // ops already recalculated
    QSet<CustomObject*> modified;       // objects whose dependents need checking
    modified.insert(modifiedObj);

    // Block signals during entire recalc cascade to avoid side-effects
    const QSignalBlocker blocker(m_currentProject);

    while (!modified.isEmpty()) {
        // Take one object and find its dependent operations
        CustomObject* obj = *modified.begin();
        modified.erase(modified.begin());

        // Rebuild so the graph knows about newly created results from previous iterations
        m_depGraph->rebuildFromProject(m_currentProject);

        QVector<CustomOperation*> ops = m_depGraph->operationsUsingObject(obj);
        for (auto* op : ops) {
            if (!op || processed.contains(op)) continue;
            processed.insert(op);

            // Remove old result by name
            QString resName = op->resultName();
            CustomObject* oldResult = m_currentProject->findObject(resName);
            if (oldResult) {
                m_currentProject->removeResultObject(oldResult);
                m_currentProject->removeDataObject(oldResult);
            }
            // Execute — creates new result
            op->execute(m_currentProject);

            // The new result now needs its dependents checked too
            CustomObject* newResult = m_currentProject->findObject(resName);
            if (newResult)
                modified.insert(newResult);
        }
    }

    // Rebuild dependency graph to reflect new results
    m_depGraph->rebuildFromProject(m_currentProject);
    m_recalcInProgress = false;
}

void MainWindow::refreshChart()
{
#ifdef HAS_QT_CHARTS
    if (!m_chart || !m_currentProject) return;
    m_chart->removeAllSeries();
    ChartWidget::populateChart(m_chart, m_currentProject, m_showControlPoints, m_showNormals, m_selectedObject);
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