#include "DependencyGraphDialog.h"
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>

namespace ExpressDesigner {

DependencyGraphDialog::DependencyGraphDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Dependency Graph"));
    setMinimumSize(500, 400);

    auto* layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel(QStringLiteral("Object → Operation → Result relationships:"), this));

    m_textEdit = new QTextEdit(this);
    m_textEdit->setReadOnly(true);
    m_textEdit->setFont(QFont(QStringLiteral("Courier New"), 9));
    layout->addWidget(m_textEdit);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void DependencyGraphDialog::setProject(Project* project, DependencyGraph* graph)
{
    if (!project || !graph) return;

    graph->rebuildFromProject(project);
    buildGraphText(project, graph);
}

void DependencyGraphDialog::buildGraphText(Project* project, DependencyGraph* graph)
{
    QString text;
    text += QStringLiteral("=== DEPENDENCY GRAPH ===\n\n");

    const auto& ops = project->operations();
    for (auto* op : ops) {
        if (!op) continue;
        text += QStringLiteral("Operation: %1\n").arg(op->name());

        for (int i = 0; i < op->paramCount(); ++i) {
            QString prefix = op->paramPrefixOnTree(i);
            QString paramName = op->paramName(i);
            if (op->isParamObject(i)) {
                CustomObject* paramObj = project->findObject(paramName);
                if (paramObj) {
                    text += QStringLiteral("  %1%2 (object)\n").arg(prefix, paramName);
                } else {
                    text += QStringLiteral("  %1%2 (missing)\n").arg(prefix, paramName);
                }
            } else {
                text += QStringLiteral("  %1%2 (value)\n").arg(prefix, paramName);
            }
        }

        CustomObject* result = graph->resultOfOperation(op);
        if (result) {
            text += QStringLiteral("  → Result: %1\n").arg(result->name());
        } else {
            text += QStringLiteral("  → Result: %1 (not found)\n").arg(op->resultName());
        }
        text += QStringLiteral("\n");
    }

    if (ops.isEmpty()) {
        text += QStringLiteral("No operations in project.\n");
    }

    m_textEdit->setText(text);
}

} // namespace ExpressDesigner