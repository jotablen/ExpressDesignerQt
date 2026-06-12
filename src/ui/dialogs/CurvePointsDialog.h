#pragma once
#include <QDialog>
#include <QTableWidget>
#include <QPushButton>

namespace ExpressDesigner {
class CurvePointsDialog : public QDialog {
    Q_OBJECT
public:
    explicit CurvePointsDialog(QWidget* parent = nullptr);
    void setReadOnly(bool ro) { m_readOnly = ro; }
    QTableWidget* grid() const { return m_grid; }
private slots:
    void onAddPoint();
    void onDeletePoint();
    void onInsertPoint();
    void onEditPoint();
    void onSave();
private:
    QTableWidget* m_grid = nullptr;
    QPushButton* m_addBtn = nullptr;
    QPushButton* m_delBtn = nullptr;
    QPushButton* m_insBtn = nullptr;
    QPushButton* m_editBtn = nullptr;
    QPushButton* m_saveBtn = nullptr;
    bool m_readOnly = false;
};
}