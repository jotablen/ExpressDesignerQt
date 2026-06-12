#include "CurvePointsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QFrame>
#include <QTableWidget>
#include <QHeaderView>

namespace ExpressDesigner {
CurvePointsDialog::CurvePointsDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(QStringLiteral("Curve points definition"));
    setMinimumWidth(370);
    setMaximumWidth(370);
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(QStringLiteral(" List of Points Coordinates "), this));
    auto* headerRow = new QHBoxLayout();
    headerRow->addStretch(1);
    headerRow->addWidget(new QLabel(QStringLiteral(" X"), this));
    headerRow->addStretch(4);
    headerRow->addWidget(new QLabel(QStringLiteral(" Y"), this));
    headerRow->addStretch(1);
    layout->addLayout(headerRow);
    m_grid = new QTableWidget(0, 2, this);
    m_grid->setHorizontalHeaderLabels({QStringLiteral("X"), QStringLiteral("Y")});
    m_grid->horizontalHeader()->setStretchLastSection(true);
    m_grid->setSelectionBehavior(QAbstractItemView::SelectRows);
    layout->addWidget(m_grid);
    auto* btnRow = new QHBoxLayout();
    m_addBtn = new QPushButton(QStringLiteral("&Add"), this);
    m_insBtn = new QPushButton(QStringLiteral("&Insert"), this);
    m_editBtn = new QPushButton(QStringLiteral("&Edit"), this);
    m_delBtn = new QPushButton(QStringLiteral("&Delete"), this);
    btnRow->addWidget(m_addBtn);
    btnRow->addWidget(m_insBtn);
    btnRow->addWidget(m_editBtn);
    btnRow->addWidget(m_delBtn);
    layout->addLayout(btnRow);
    m_saveBtn = new QPushButton(QStringLiteral("Save"), this);
    auto* cancelBtn = new QPushButton(QStringLiteral("Cancel"), this);
    auto* btnRow2 = new QHBoxLayout();
    btnRow2->addStretch();
    btnRow2->addWidget(m_saveBtn);
    btnRow2->addWidget(cancelBtn);
    layout->addLayout(btnRow2);
    connect(m_addBtn, &QPushButton::clicked, this, &CurvePointsDialog::onAddPoint);
    connect(m_delBtn, &QPushButton::clicked, this, &CurvePointsDialog::onDeletePoint);
    connect(m_insBtn, &QPushButton::clicked, this, &CurvePointsDialog::onInsertPoint);
    connect(m_editBtn, &QPushButton::clicked, this, &CurvePointsDialog::onEditPoint);
    connect(m_saveBtn, &QPushButton::clicked, this, &CurvePointsDialog::onSave);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}
void CurvePointsDialog::onAddPoint() { m_grid->insertRow(m_grid->rowCount()); }
void CurvePointsDialog::onDeletePoint() {
    int row = m_grid->currentRow();
    if (row >= 0) m_grid->removeRow(row);
}
void CurvePointsDialog::onInsertPoint() {
    int row = m_grid->currentRow();
    m_grid->insertRow(row >= 0 ? row : 0);
}
void CurvePointsDialog::onEditPoint() { m_grid->editItem(m_grid->currentItem()); }
void CurvePointsDialog::onSave() { accept(); }
}