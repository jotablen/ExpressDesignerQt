#include "MainWindow.h"
#include <core/ObjectFactory.h>
#include <ui/dialogs/InsertObjectDialog.h>
#include <ui/dialogs/CalcOvalDialog.h>
#include <ui/dialogs/PropagateWFDialog.h>
#include <ui/dialogs/OffsetWFDialog.h>
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
#ifdef HAS_QT_CHARTS
#include <QtCharts/QLineSeries>
#endif

namespace ExpressDesigner {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_history(new HistoryManager)
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
    editMenu->addAction(tr("&Preferences..."), this, []() {});

    // Tasks menu
    QMenu* tasksMenu = menuBar()->addMenu(tr("&Tasks"));
    tasksMenu->addAction(tr("&Calculate Oval..."), this, &MainWindow::onCalculateOval);
    tasksMenu->addAction(tr("&Propagate WF..."), this, &MainWindow::onPropagateWF);
    tasksMenu->addAction(tr("&Offset WF..."), this, &MainWindow::onOffsetWF);
    tasksMenu->addAction(tr("&Recalculate All"), this, &MainWindow::onRecalculate);

    // View menu
    QMenu* viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(tr("Zoom &In"), QKeySequence::ZoomIn, this, &MainWindow::onZoomIn);
    viewMenu->addAction(tr("Zoom &Out"), QKeySequence::ZoomOut, this, &MainWindow::onZoomOut);
    viewMenu->addAction(tr("Zoom &All"), this, &MainWindow::onZoomAll);
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
    toolbar->addSeparator();
    toolbar->addAction(tr("Delete"), this, &MainWindow::onDeleteObject);
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
    // Install event filter to capture mouse clicks on chart
    m_chartView->viewport()->installEventFilter(this);
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
    // Chart gets all extra space, properties anchored to bottom
    m_rightSplitter->setStretchFactor(0, 1);
    m_rightSplitter->setStretchFactor(1, 0);
    m_rightSplitter->setCollapsible(1, false);
    // Set initial sizes: chart big, properties at default height
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

    // Refresh chart when properties are modified
    connect(m_propertiesWidget, &PropertiesWidget::objectModified,
            this, [this](CustomObject*) {
                setModified(true);
                refreshChart();
                updateStatusBar();
            });
    connect(m_propertiesWidget, &PropertiesWidget::projectModified,
            this, [this](Project*) {
                setModified(true);
                refreshChart();
                updateStatusBar();
            });

    // Context menu on object tree (matching Ovals Designer popupMenuChart)
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
            if (obj) {
                obj->setVisible(!obj->isVisible());
                setModified(true);
                refreshChart();
            }
        }
        else if (chosen == togglePtsAct) onToggleControlPoints();
        else if (chosen == zoomInAct) onZoomIn();
        else if (chosen == zoomOutAct) onZoomOut();
        else if (chosen == zoomAllAct) onZoomAll();
    });
}

// Event filter to intercept mouse clicks on the chart viewport
bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
#ifdef HAS_QT_CHARTS
    if (m_chartView && watched == m_chartView->viewport()
        && event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        // Map viewport coords to chart scene coords
        QPointF chartPos = m_chartView->mapToScene(me->pos());
        onChartClicked(chartPos);

        // Find clicked series and select its associated object
        CustomObject* clickedObj = nullptr;
        const auto seriesList = m_chart->series();
        double bestDist = 10.0; // snap distance in chart coords
        for (auto* ser : seriesList) {
            QLineSeries* ls = qobject_cast<QLineSeries*>(ser);
            if (!ls) continue;
            // Only consider main curve series (named, not normals)
            QString name = ls->name();
            if (name.isEmpty()) continue;
            // Check each point for proximity
            for (int i = 0; i < ls->count(); ++i) {
                QPointF pt = ls->at(i);
                double dx = pt.x() - chartPos.x();
                double dy = pt.y() - chartPos.y();
                double dist = qSqrt(dx * dx + dy * dy);
                if (dist < bestDist) {
                    bestDist = dist;
                    QVariant prop = ls->property("customObject");
                    if (prop.isValid()) {
                        clickedObj = reinterpret_cast<CustomObject*>(prop.value<quintptr>());
                    }
                }
            }
        }

        if (clickedObj && m_treeModel) {
            // Find and select the object in the tree
            QModelIndex idx = m_treeModel->createIndexByObject(clickedObj);
            if (idx.isValid()) {
                m_objectTree->selectionModel()->setCurrentIndex(
                    idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
            }
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
    if (!loaded) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to load project."));
        return;
    }
    m_currentFilePath = filePath;
    setProject(loaded);
    m_history->addEntry(QStringLiteral("project_opened"), filePath);
}

void MainWindow::onSaveProject()
{
    if (m_currentFilePath.isEmpty()) {
        onSaveProjectAs();
        return;
    }
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
    if (dlg.exec() == QDialog::Accepted) {
        if (m_currentProject) {
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
}

void MainWindow::onDeleteObject()
{
    QModelIndex idx = m_objectTree->currentIndex();
    if (!idx.isValid() || !m_currentProject) return;

    CustomObject* obj = m_treeModel->objectAt(idx);
    if (!obj) return;

    // Try data objects first, then result objects
    int dataIdx = m_currentProject->findObjectIndex(obj);
    if (dataIdx >= 0 && dataIdx < m_currentProject->dataObjectCount()) {
        m_currentProject->removeDataObject(dataIdx);
    } else {
        m_currentProject->removeResultObject(obj);
    }
    m_history->recordObjectDeletion(obj->name());
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
    // TXTImporter not yet implemented
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

void MainWindow::onExportAllRhino()
{
    // ExportAllRhinoDialog - stub not yet implemented
}

void MainWindow::onCalculateOval()
{
    if (!m_currentProject) return;
    CalcOvalDialog dlg(this);
    dlg.setProject(m_currentProject);
    if (dlg.exec() == QDialog::Accepted) {
        auto* op = new CarthesianOvalOperation(dlg.resultNameEdit()->text(), this);
        op->setAmountOfPoints(dlg.amountEdit()->text().toInt());
        // Set param names from selected combos
        op->setParamName(CarthesianOvalOperation::PARAM_WF1, dlg.wfOriginCombo()->currentText());
        op->setParamName(CarthesianOvalOperation::PARAM_WF2, dlg.wfDestCombo()->currentText());
        op->setParamName(CarthesianOvalOperation::PARAM_REF_POINT, dlg.refPointCombo()->currentText());
        if (op->execute(m_currentProject)) {
            m_currentProject->addOperation(op);
            m_history->recordObjectCreation(op->name());
            setModified(true);
        } else {
            delete op;
        }
        refreshChart();
        updateStatusBar();
    }
}

void MainWindow::onPropagateWF()
{
    if (!m_currentProject) return;
    PropagateWFDialog dlg(this);
    dlg.setProject(m_currentProject);
    if (dlg.exec() == QDialog::Accepted) {
        auto* op = new PropagateWFOperation(dlg.resultNameEdit()->text(), this);
        op->setAmountOfPoints(dlg.amountEdit()->text().toInt());
        op->setOffset(dlg.offsetEdit()->text().toDouble());
        op->setParamName(PropagateWFOperation::PARAM_WF, dlg.wfOrgCombo()->currentText());
        op->setParamName(PropagateWFOperation::PARAM_SURFACE, dlg.wfDestCombo()->currentText());
        op->setParamName(PropagateWFOperation::PARAM_IOR, dlg.indexDestEdit()->text());
        if (op->execute(m_currentProject)) {
            m_currentProject->addOperation(op);
            m_history->recordObjectCreation(op->name());
            setModified(true);
        } else {
            delete op;
        }
        refreshChart();
        updateStatusBar();
    }
}

void MainWindow::onOffsetWF()
{
    if (!m_currentProject) return;
    OffsetWFDialog dlg(this);
    dlg.setProject(m_currentProject);
    if (dlg.exec() == QDialog::Accepted) {
        CustomObject* srcWf = nullptr;
        QString wfName = dlg.wfCombo()->currentText();
        srcWf = m_currentProject->findObject(wfName);
        if (!srcWf) return;

        double offset = dlg.offsetEdit()->text().toDouble();
        QString resultName;
        if (dlg.createNewResult()) {
            resultName = dlg.resultNameEdit()->text().trimmed();
            if (resultName.isEmpty()) resultName = wfName + QStringLiteral("_offset");
        } else {
            resultName = wfName;
        }

        auto* result = new CurveObject(resultName);
        result->setObjectType(withResult(ObjectType::Curve));
        result->setRefractiveIndex(srcWf->refractiveIndex());

        QVector<QPointF> offsetPts;
        const auto& srcPts = srcWf->controlPoints();
        offsetPts.reserve(srcPts.size());
        for (const auto& pt : srcPts)
            offsetPts.append(QPointF(pt.x() + offset, pt.y()));

        result->setControlPoints(offsetPts);
        m_currentProject->addResultObject(result);

        setModified(true);
        m_history->recordObjectCreation(resultName);
        refreshChart();
        updateStatusBar();
    }
}

void MainWindow::onRecalculate()
{
    refreshChart();
}

void MainWindow::onZoomIn()
{
#ifdef HAS_QT_CHARTS
    if (m_chart) m_chart->zoomIn();
#endif
}

void MainWindow::onZoomOut()
{
#ifdef HAS_QT_CHARTS
    if (m_chart) m_chart->zoomOut();
#endif
}

void MainWindow::onZoomAll()
{
#ifdef HAS_QT_CHARTS
    if (m_chart) m_chart->zoomReset();
#endif
}

void MainWindow::onToggleControlPoints()
{
    m_showControlPoints = !m_showControlPoints;
    refreshChart();
}

void MainWindow::onToggleLabels()
{
    m_showLabels = !m_showLabels;
    refreshChart();
}

void MainWindow::onToggleNormals()
{
    m_showNormals = !m_showNormals;
    refreshChart();
}

void MainWindow::onSetAspectRatio() {}

void MainWindow::onObjectSelected(const QModelIndex& index)
{
    m_selectedObject = m_treeModel->objectAt(index);
    if (m_propertiesWidget)
        m_propertiesWidget->setObject(m_selectedObject);
    refreshChart();
    updateStatusBar();
}

void MainWindow::onChartClicked(const QPointF& point)
{
    statusBar()->showMessage(tr("Position: (%1, %2)")
        .arg(point.x(), 0, 'f', 2)
        .arg(point.y(), 0, 'f', 2));
}

void MainWindow::onAbout()
{
    AboutDialog dlg(this);
    dlg.exec();
}

void MainWindow::onShowHistory()
{
    // ProjectHistoryDialog - stub not yet implemented
}

void MainWindow::refreshChart()
{
#ifdef HAS_QT_CHARTS
    if (!m_chart || !m_currentProject) return;
    m_chart->removeAllSeries();
    ChartWidget::populateChart(m_chart, m_currentProject, m_showControlPoints, m_showNormals,
                               m_selectedObject);
#endif
}

void MainWindow::updateStatusBar()
{
    if (!m_currentProject) {
        statusBar()->showMessage(tr("No project open"));
        return;
    }
    int totalObjects = m_currentProject->dataObjectCount() + m_currentProject->resultObjectCount();
    statusBar()->showMessage(tr("Ready | Objects: %1").arg(totalObjects));
}

void MainWindow::setModified(bool modified)
{
    m_isModified = modified;
    if (modified)
        setWindowTitle(QStringLiteral("ExpressDesigner *"));
    else
        setWindowTitle(QStringLiteral("ExpressDesigner"));
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (m_isModified) {
        auto answer = QMessageBox::question(this, tr("Unsaved Changes"),
            tr("Save changes before exiting?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (answer == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
        if (answer == QMessageBox::Save) onSaveProject();
    }

    QSettings settings;
    settings.setValue(QStringLiteral("MainWindow/geometry"), saveGeometry());
    settings.setValue(QStringLiteral("MainWindow/state"), saveState());
    event->accept();
}

} // namespace ExpressDesigner