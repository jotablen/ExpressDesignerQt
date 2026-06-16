#include "DependencyGraphDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QHeaderView>

namespace ExpressDesigner {

DependencyGraphDialog::DependencyGraphDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Dependency Graph"));
    setMinimumSize(550, 450);

    auto* layout = new QVBoxLayout(this);

    auto* headerLayout = new QHBoxLayout();
    headerLayout->addWidget(new QLabel(QStringLiteral("Object / Operation tree:"), this));
    headerLayout->addStretch();
    m_refreshBtn = new QPushButton(QStringLiteral("&Refresh"), this);
    headerLayout->addWidget(m_refreshBtn);
    layout->addLayout(headerLayout);

    m_tree = new QTreeWidget(this);
    m_tree->setHeaderLabels({QStringLiteral("Name"), QStringLiteral("Type")});
    m_tree->setAlternatingRowColors(true);
    m_tree->setRootIsDecorated(true);
    m_tree->header()->setStretchLastSection(false);
    m_tree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_tree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    layout->addWidget(m_tree);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_refreshBtn, &QPushButton::clicked, this, &DependencyGraphDialog::onRefresh);
}

void DependencyGraphDialog::setProject(Project* project, DependencyGraph* graph)
{
    m_project = project;
    m_graph = graph;
    onRefresh();
}

void DependencyGraphDialog::onRefresh()
{
    if (m_graph && m_project)
        m_graph->rebuildFromProject(m_project);
    buildGraph();
}

void DependencyGraphDialog::buildGraph()
{
    m_tree->clear();
    if (!m_project) return;

    // Object → uses in operations → results
    // Show structure: each operation lists its input objects, then its result
    const auto& ops = m_project->operations();

    for (auto* op : ops) {
        if (!op) continue;

        auto* opItem = new QTreeWidgetItem(m_tree);
        opItem->setText(0, op->name());
        opItem->setText(1, QStringLiteral("Operation"));
        opItem->setExpanded(true);

        // Show input parameters
        for (int i = 0; i < op->paramCount(); ++i) {
            QString paramName = op->paramName(i);
            QString prefix = op->paramPrefixOnTree(i);
            auto* paramItem = new QTreeWidgetItem(opItem);
            paramItem->setText(0, prefix + paramName);
            if (op->isParamObject(i)) {
                CustomObject* paramObj = m_project->findObject(paramName);
                paramItem->setText(1, paramObj ? QStringLiteral("Object ✓") : QStringLiteral("Object (missing)"));
            } else {
                paramItem->setText(1, QStringLiteral("Value"));
            }
        }

        // Show result
        auto* resultItem = new QTreeWidgetItem(opItem);
        CustomObject* result = nullptr;
        if (m_graph)
            result = m_graph->resultOfOperation(op);
        if (!result)
            result = m_project->findObject(op->resultName());

        resultItem->setText(0, result ? result->name() : op->resultName());
        resultItem->setText(1, result ? QStringLiteral("Result ✓") : QStringLiteral("Result (not found)"));
    }

    if (ops.isEmpty()) {
        auto* emptyItem = new QTreeWidgetItem(m_tree);
        emptyItem->setText(0, QStringLiteral("No operations in project"));
        emptyItem->setText(1, QStringLiteral(""));
    }
}

} // namespace ExpressDesigner