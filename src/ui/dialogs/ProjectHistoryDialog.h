#pragma once
#include <QDialog>
#include <QTextEdit>

namespace ExpressDesigner {
class ProjectHistoryDialog : public QDialog {
    Q_OBJECT
public:
    explicit ProjectHistoryDialog(QWidget* parent = nullptr);
    void setHistory(const QString& text);
private slots:
    void onClear();
private:
    QTextEdit* m_historyEdit = nullptr;
};
}