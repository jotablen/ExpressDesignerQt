#pragma once
#include <QDialog>
#include <QTreeWidget>
#include <QPushButton>
#include <core/Project.h>
#include <core/DependencyGraph.h>

namespace ExpressDesigner {

class DependencyGraphDialog : public QDialog {
    Q_OBJECT
public:
    explicit DependencyGraphDialog(QWidget* parent = nullptr);
    void setProject(Project* project, DependencyGraph* graph);

private slots:
    void onRefresh();

private:
    void buildGraph();

    QTreeWidget* m_tree = nullptr;
    QPushButton* m_refreshBtn = nullptr;
    Project* m_project = nullptr;
    DependencyGraph* m_graph = nullptr;
};

} // namespace ExpressDesigner