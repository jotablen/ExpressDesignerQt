#include "ProjectHistoryDialog.h"
#include <QVBoxLayout>
#include <QPushButton>

namespace ExpressDesigner {
ProjectHistoryDialog::ProjectHistoryDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(QStringLiteral("Project commands history..."));
    setMinimumSize(300, 300);
    auto* layout = new QVBoxLayout(this);
    m_historyEdit = new QTextEdit(this);
    m_historyEdit->setReadOnly(true);
    m_historyEdit->setFont(QFont(QStringLiteral("MS Sans Serif"), 11));
    layout->addWidget(m_historyEdit);
    auto* clearBtn = new QPushButton(QStringLiteral("Clear command history..."), this);
    auto* closeBtn = new QPushButton(QStringLiteral("Close"), this);
    auto* btnRow = new QHBoxLayout();
    btnRow->addWidget(clearBtn);
    btnRow->addStretch();
    btnRow->addWidget(closeBtn);
    layout->addLayout(btnRow);
    connect(clearBtn, &QPushButton::clicked, this, &ProjectHistoryDialog::onClear);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
}
void ProjectHistoryDialog::setHistory(const QString& text) { m_historyEdit->setPlainText(text); }
void ProjectHistoryDialog::onClear() { m_historyEdit->clear(); }
}