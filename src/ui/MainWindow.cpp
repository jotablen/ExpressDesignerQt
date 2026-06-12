#include "MainWindow.h"
#include <core/ObjectFactory.h>
#include <ui/dialogs/InsertObjectDialog.h>
#include <ui/dialogs/CalcOvalDialog.h>
#include <ui/dialogs/PropagateWFDialog.h>
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
    m_chartView->setChart(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
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
    m_rightSplitter->setStretchFactor(0, 2);
    m_rightSplitter->setStretchFactor(1, 1);

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

    m_currentProject->removeDataObject(obj);
    m_history->recordObjectDeletion(obj->name());
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
        refreshChart();
        updateStatusBar();
    }
}

void MainWindow::onOffsetWF()
{
    // Will be implemented with OffsetWFDialog
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
}

void MainWindow::onToggleNormals()
{
    m_showNormals = !m_showNormals;
    refreshChart();
}

void MainWindow::onSetAspectRatio() {}

void MainWindow::onObjectSelected(const QModelIndex& index)
{
    CustomObject* obj = m_treeModel->objectAt(index);
    if (m_propertiesWidget)
        m_propertiesWidget->setObject(obj);
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
    ChartWidget::populateChart(m_chart, m_currentProject, m_showControlPoints, m_showNormals);
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