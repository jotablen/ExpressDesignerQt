#pragma once

#include <QMainWindow>
#include <QCloseEvent>
#include <QEvent>
#include <QTreeView>
#include <QPoint>
#ifdef HAS_QT_CHARTS
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#endif
#include <QTabWidget>
#include <QSplitter>
#include <QToolBar>
#include <QStatusBar>
#include <QMenuBar>
#include <QAction>
#include <QMap>

#include <core/Project.h>
#include <core/ObjectTypes.h>
#include <core/DependencyGraph.h>
#include <core/CommandHistory.h>
#include <io/HistoryManager.h>
#include <ui/models/ObjectTreeModel.h>

namespace ExpressDesigner {

class ObjectTreeWidget;
class PropertiesWidget;
class ChartWidget;
class HistoryManager;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    void setProject(Project* project);
    Project* currentProject() const { return m_currentProject; }

private slots:
    // File menu
    void onNewProject();
    void onOpenProject();
    void onSaveProject();
    void onSaveProjectAs();
    void onCloseProject();

    // Object actions
    void onInsertObject();
    void onDeleteObject();
    void onEditObject();
    void onImportObject();
    void onExportObject();
    void onExportCAD();
    void onExportAllRhino();

    // Calculation actions
    void onCalculateOval();
    void onPropagateWF();
    void onOffsetWF();
    void onRecalculate();

    // Transform actions
    void onRotateObject();
    void onTranslateObject();
    void onCopyObject();

    // View actions
    void onZoomIn();
    void onZoomOut();
    void onZoomAll();
    void onShowDependencyGraph();
    void onToggleControlPoints();
    void onToggleLabels();
    void onToggleNormals();
    void onSetAspectRatio();

    // Edit
    void onPreferences();
    void onUndo();
    void onRedo();

    // Selection
    void onObjectSelected(const QModelIndex& index);
    void onChartClicked(const QPointF& point);

    // Help
    void onAbout();
    void onShowHistory();

protected:
    void closeEvent(QCloseEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupCentralWidget();
    void setupConnections();
    void refreshChart();
    void updateStatusBar();
    void setModified(bool modified);
    void updateDeleteActionState();
    void updateUndoRedoActions();

    // Cascade recalculation — re-executes all operations in order
    void recalculateAll();

    // Widgets
    QTreeView* m_objectTree = nullptr;
#ifdef HAS_QT_CHARTS
    QChartView* m_chartView = nullptr;
#endif
    PropertiesWidget* m_propertiesWidget = nullptr;
    QSplitter* m_mainSplitter = nullptr;
    QSplitter* m_rightSplitter = nullptr;

    // Actions
    QAction* m_deleteAction = nullptr;
    QAction* m_undoAction = nullptr;
    QAction* m_redoAction = nullptr;

    // Models
    ObjectTreeModel* m_treeModel = nullptr;

    // Project
    Project* m_currentProject = nullptr;

    // Chart
#ifdef HAS_QT_CHARTS
    QChart* m_chart = nullptr;
#endif
    CustomObject* m_selectedObject = nullptr;
    CustomOperation* m_selectedOperation = nullptr;

    // Core systems
    HistoryManager* m_history = nullptr;
    DependencyGraph* m_depGraph = nullptr;
    CommandHistory* m_cmdHistory = nullptr;

    // Settings
    bool m_autoZoomOnInsert = true;
    bool m_showControlPoints = true;
    bool m_showLabels = true;
    bool m_showNormals = false;
    bool m_isModified = false;

    // Pan state
    bool m_isPanning = false;
    QPoint m_panLastPos;

    // Control-point drag state (Ctrl + drag)
    bool m_isDraggingCP = false;
    int m_dragCPIndex = -1;
    QVector<QPointF> m_dragCPOldPoints; // snapshot before drag starts


    // Chart helpers
    void maintainChartAspectRatio();

    // Chart viewport event handlers (extracted from eventFilter)
    bool handleViewportWheel(QWheelEvent* we);
    bool handleViewportMousePress(QMouseEvent* me);
    bool handleViewportMouseMove(QMouseEvent* me);
    bool handleViewportMouseRelease(QMouseEvent* me);
    // Click-select: find nearest/second-nearest object and select in tree
    void performChartClickSelect(const QPointF& chartPos);
    // Compute snap distance as a percentage of the visible chart range
    double snapDistance(double percent) const;

    QString m_currentFilePath;
};

} // namespace ExpressDesigner