#pragma once

#include <QMainWindow>
#include <QCloseEvent>
#include <QEvent>
#include <QTreeView>
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
    void onExportAllRhino();

    // Calculation actions
    void onCalculateOval();
    void onPropagateWF();
    void onOffsetWF();
    void onRecalculate();

    // View actions
    void onZoomIn();
    void onZoomOut();
    void onZoomAll();
    void onToggleControlPoints();
    void onToggleLabels();
    void onToggleNormals();
    void onSetAspectRatio();

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

    // Widgets
    QTreeView* m_objectTree = nullptr;
#ifdef HAS_QT_CHARTS
    QChartView* m_chartView = nullptr;
#endif
    PropertiesWidget* m_propertiesWidget = nullptr;
    QSplitter* m_mainSplitter = nullptr;
    QSplitter* m_rightSplitter = nullptr;

    // Models
    ObjectTreeModel* m_treeModel = nullptr;

    // Project
    Project* m_currentProject = nullptr;

    // Chart
#ifdef HAS_QT_CHARTS
    QChart* m_chart = nullptr;
#endif
    CustomObject* m_selectedObject = nullptr;

    // History
    HistoryManager* m_history = nullptr;

    // Settings
    bool m_autoZoomOnInsert = true;
    bool m_showControlPoints = true;
    bool m_showLabels = true;
    bool m_showNormals = false;
    bool m_isModified = false;

    QString m_currentFilePath;
};

} // namespace ExpressDesigner