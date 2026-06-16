#pragma once
#include <QDialog>
#include <QTextEdit>
#include <core/Project.h>
#include <core/DependencyGraph.h>

namespace ExpressDesigner {

class DependencyGraphDialog : public QDialog {
    Q_OBJECT
public:
    explicit DependencyGraphDialog(QWidget* parent = nullptr);
    void setProject(Project* project, DependencyGraph* graph);

private:
    void buildGraphText(Project* project, DependencyGraph* graph);

    QTextEdit* m_textEdit = nullptr;
};

} // namespace ExpressDesigner