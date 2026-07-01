#include "PropertiesWidget.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFrame>
#include <QHeaderView>
#include <QMessageBox>
#include <QScrollArea>
#include <core/ObjectTypes.h>
#include <core/ArcObject.h>
#include <core/PointObject.h>
#include <core/LineObject.h>
#include <core/CurveObject.h>
#include <core/PropagateWFOperation.h>
#include <ui/dialogs/CurvePointsDialog.h>

namespace ExpressDesigner {

// Wrap a tab page in a scroll area so its content stays accessible
// even when the properties panel is short (matches ODs behaviour where
// the page content has a fixed height and the panel scrolls).
static QScrollArea* wrapScroll(QWidget* page)
{
    auto* scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidget(page);
    return scroll;
}

PropertiesWidget::PropertiesWidget(QWidget* parent) : QWidget(parent)
{
    m_tabs = new QTabWidget(this);
    m_tabs->setTabPosition(QTabWidget::South);
    m_tabs->tabBar()->setVisible(false);
    m_tabs->setStyleSheet(QStringLiteral("QTabWidget::pane { border: 1px solid #C4C4C3; }"));
    setupProjectTab(); setupPointTab(); setupLineTab(); setupArcTab(); setupCurveTab();
    setupObjectsTab(); setupCalcOvalTab(); setupPropagateTab();
    auto* layout = new QVBoxLayout(this); layout->setContentsMargins(0, 0, 0, 0); layout->addWidget(m_tabs);
    m_tabs->setCurrentIndex(0);

    // Dynamic default button: set active tab's Save as default on tab change
    connect(m_tabs, &QTabWidget::currentChanged, this, [this](int index) {
        // Walk all tab pages, find their Save buttons, and update default state
        for (int i = 0; i < m_tabs->count(); ++i) {
            QWidget* page = m_tabs->widget(i);
            if (!page) continue;
            QList<QPushButton*> buttons = page->findChildren<QPushButton*>();
            for (auto* btn : buttons) {
                if (btn->text() == QStringLiteral("Save"))
                    btn->setDefault(i == index);
            }
        }
    });
    // Set initial default for the project tab (index 0)
    QWidget* initPage = m_tabs->widget(0);
    if (initPage) {
        QList<QPushButton*> buttons = initPage->findChildren<QPushButton*>();
        for (auto* btn : buttons) {
            if (btn->text() == QStringLiteral("Save")) btn->setDefault(true);
        }
    }
}

void PropertiesWidget::setupProjectTab() {
    auto* tab = new QWidget(); auto* l = new QVBoxLayout(tab);
    auto* nr = new QHBoxLayout(); auto* nl = new QLabel(QStringLiteral("Project Name"), tab);
    QFont bf = nl->font(); bf.setBold(true); nl->setFont(bf); nr->addWidget(nl); l->addLayout(nr);
    m_prjNameEdit = new QLineEdit(tab); l->addWidget(m_prjNameEdit);
    auto* xg = new QGroupBox(QStringLiteral(" X Axis Scale "), tab); xg->setMinimumWidth(150); xg->setMaximumWidth(400); auto* xv = new QVBoxLayout(xg); auto* xl = new QFormLayout(); xv->addLayout(xl);
    m_prjXMinEdit = new QLineEdit(QStringLiteral("0"), xg); m_prjXMinEdit->setEnabled(false); xl->addRow(QStringLiteral("&Min:"), m_prjXMinEdit);
    m_prjXMaxEdit = new QLineEdit(QStringLiteral("0"), xg); m_prjXMaxEdit->setEnabled(false); xl->addRow(QStringLiteral("M&ax:"), m_prjXMaxEdit);
    m_prjXAutoCheck = new QCheckBox(QStringLiteral("Auto scale X axis"), xg); m_prjXAutoCheck->setEnabled(false); xv->addWidget(m_prjXAutoCheck);
    auto* yg = new QGroupBox(QStringLiteral(" Y Axis Scale "), tab); yg->setMinimumWidth(150); yg->setMaximumWidth(400); auto* yv = new QVBoxLayout(yg); auto* yl = new QFormLayout(); yv->addLayout(yl);
    m_prjYMinEdit = new QLineEdit(QStringLiteral("0"), yg); m_prjYMinEdit->setEnabled(false); yl->addRow(QStringLiteral("&Min:"), m_prjYMinEdit);
    m_prjYMaxEdit = new QLineEdit(QStringLiteral("0"), yg); m_prjYMaxEdit->setEnabled(false); yl->addRow(QStringLiteral("M&ax:"), m_prjYMaxEdit);
    m_prjYAutoCheck = new QCheckBox(QStringLiteral("Auto scale Y axis"), yg); m_prjYAutoCheck->setEnabled(false); yv->addWidget(m_prjYAutoCheck);
    auto* scaleRow = new QHBoxLayout();
    scaleRow->addWidget(xg, 1);
    scaleRow->addWidget(yg, 1);
    l->addLayout(scaleRow);
    auto* br = new QHBoxLayout(); auto* sb = new QPushButton(QStringLiteral("Save"), tab); sb->setDefault(true);
    auto* rb = new QPushButton(QStringLiteral("Restore"), tab); br->addStretch(); br->addWidget(rb); br->addWidget(sb); l->addStretch(); l->addLayout(br);
    m_tabs->addTab(wrapScroll(tab), QStringLiteral("Project"));
    connect(sb, &QPushButton::clicked, this, &PropertiesWidget::onSaveProject);
    connect(rb, &QPushButton::clicked, this, &PropertiesWidget::onRestoreProject);
}

QGroupBox* PropertiesWidget::createWFPanel(QWidget* parent, QLineEdit*& refIndexEdit, QCheckBox*& flipNCheck)
{
    auto* panel = new QGroupBox(QStringLiteral(" WF Parameters "), parent);
    panel->setMaximumWidth(160);
    auto* wfCol = new QVBoxLayout(panel);
    wfCol->addWidget(new QLabel(QStringLiteral("Refractive Index:"), panel));
    refIndexEdit = new QLineEdit(QStringLiteral("0"), panel);
    wfCol->addWidget(refIndexEdit);
    flipNCheck = new QCheckBox(QStringLiteral("Flip normals"), panel);
    wfCol->addWidget(flipNCheck);
    return panel;
}

void PropertiesWidget::setupPointTab() {
    auto* tab = new QWidget(); auto* l = new QVBoxLayout(tab);
    auto* nr = new QHBoxLayout(); auto* nl = new QLabel(QStringLiteral("Point Name"), tab);
    QFont bf = nl->font(); bf.setBold(true); nl->setFont(bf); nr->addWidget(nl);
    m_ptWFStatusLabel = new QLabel(QStringLiteral("(Non WF)"), tab); QFont sf = m_ptWFStatusLabel->font(); sf.setBold(true); m_ptWFStatusLabel->setFont(sf);
    m_ptWFStatusLabel->setStyleSheet(QStringLiteral("color: navy;")); nr->addStretch(); nr->addWidget(m_ptWFStatusLabel); l->addLayout(nr);
    m_ptNameEdit = new QLineEdit(tab); l->addWidget(m_ptNameEdit);
    auto* pg = new QGroupBox(QStringLiteral(" Point Coordinates "), tab); auto* pl = new QFormLayout(pg);
    m_ptXEdit = new QLineEdit(QStringLiteral("0"), pg); pl->addRow(QStringLiteral(" &X:"), m_ptXEdit);
    m_ptYEdit = new QLineEdit(QStringLiteral("0"), pg); pl->addRow(QStringLiteral(" &Y:"), m_ptYEdit); l->addWidget(pg);
    auto* ptContent = new QHBoxLayout();
    m_ptWFPanel = createWFPanel(tab, m_ptRefIndexEdit, m_ptFlipNCheck);
    ptContent->addWidget(m_ptWFPanel);
    auto* ptRight = new QVBoxLayout();
    ptRight->addWidget(pg);
    ptContent->addLayout(ptRight, 1);
    l->addLayout(ptContent);
    auto* br = new QHBoxLayout(); auto* sb = new QPushButton(QStringLiteral("Save"), tab); sb->setDefault(true);
    auto* rb = new QPushButton(QStringLiteral("Restore"), tab); br->addStretch(); br->addWidget(rb); br->addWidget(sb); l->addStretch(); l->addLayout(br);
    m_tabs->addTab(wrapScroll(tab), QStringLiteral("Point"));
    connect(sb, &QPushButton::clicked, this, &PropertiesWidget::onSavePoint);
    connect(rb, &QPushButton::clicked, this, &PropertiesWidget::onRestorePoint);
}

void PropertiesWidget::setupLineTab() {
    auto* tab = new QWidget(); auto* l = new QVBoxLayout(tab);
    auto* nr = new QHBoxLayout(); auto* nl = new QLabel(QStringLiteral("Line Name"), tab);
    QFont bf = nl->font(); bf.setBold(true); nl->setFont(bf); nr->addWidget(nl);
    m_lnWFStatusLabel = new QLabel(QStringLiteral("(Non WF)"), tab); QFont sf = m_lnWFStatusLabel->font(); sf.setBold(true); m_lnWFStatusLabel->setFont(sf);
    m_lnWFStatusLabel->setStyleSheet(QStringLiteral("color: navy;")); nr->addStretch(); nr->addWidget(m_lnWFStatusLabel); l->addLayout(nr);
    m_lnNameEdit = new QLineEdit(tab); l->addWidget(m_lnNameEdit);
    auto* ptsRow = new QHBoxLayout();
    auto* p1g = new QGroupBox(QStringLiteral(" First Point Coordinates "), tab); p1g->setMinimumWidth(150); auto* p1l = new QFormLayout(p1g);
    m_lnP1XEdit = new QLineEdit(QStringLiteral("0"), p1g); p1l->addRow(QStringLiteral(" &X:"), m_lnP1XEdit);
    m_lnP1YEdit = new QLineEdit(QStringLiteral("0"), p1g); p1l->addRow(QStringLiteral(" &Y:"), m_lnP1YEdit);
    ptsRow->addWidget(p1g, 1);
    auto* p2g = new QGroupBox(QStringLiteral(" Second Point Coordinates "), tab); p2g->setMinimumWidth(150); auto* p2l = new QFormLayout(p2g);
    m_lnP2XEdit = new QLineEdit(QStringLiteral("0"), p2g); p2l->addRow(QStringLiteral(" &X:"), m_lnP2XEdit);
    m_lnP2YEdit = new QLineEdit(QStringLiteral("0"), p2g); p2l->addRow(QStringLiteral(" &Y:"), m_lnP2YEdit);
    ptsRow->addWidget(p2g, 1);
    auto* lnContent = new QHBoxLayout();
    m_lnWFPanel = createWFPanel(tab, m_lnRefIndexEdit, m_lnFlipNCheck);
    lnContent->addWidget(m_lnWFPanel);
    lnContent->addLayout(ptsRow, 1);
    l->addLayout(lnContent);
    auto* br = new QHBoxLayout(); auto* sb = new QPushButton(QStringLiteral("Save"), tab); sb->setDefault(true);
    auto* rb = new QPushButton(QStringLiteral("Restore"), tab); br->addStretch(); br->addWidget(rb); br->addWidget(sb); l->addStretch(); l->addLayout(br);
    m_tabs->addTab(wrapScroll(tab), QStringLiteral("Line"));
    connect(sb, &QPushButton::clicked, this, &PropertiesWidget::onSaveLine);
    connect(rb, &QPushButton::clicked, this, &PropertiesWidget::onRestoreLine);
}

void PropertiesWidget::setupArcTab() {
    auto* tab = new QWidget(); auto* l = new QVBoxLayout(tab);
    auto* nr = new QHBoxLayout(); auto* nl = new QLabel(QStringLiteral("Arc Name"), tab);
    QFont bf = nl->font(); bf.setBold(true); nl->setFont(bf); nr->addWidget(nl);
    m_arcWFStatusLabel = new QLabel(QStringLiteral("(Non WF)"), tab); QFont sf = m_arcWFStatusLabel->font(); sf.setBold(true); m_arcWFStatusLabel->setFont(sf);
    m_arcWFStatusLabel->setStyleSheet(QStringLiteral("color: navy;")); nr->addStretch(); nr->addWidget(m_arcWFStatusLabel); l->addLayout(nr);
    m_arcNameEdit = new QLineEdit(tab); l->addWidget(m_arcNameEdit);
    auto* arcRow = new QHBoxLayout();
    auto* cg = new QGroupBox(QStringLiteral(" Center Point Coordinates "), tab); cg->setMinimumWidth(150); auto* cl = new QFormLayout(cg);
    m_arcXEdit = new QLineEdit(QStringLiteral("0"), cg); cl->addRow(QStringLiteral(" &X:"), m_arcXEdit);
    m_arcYEdit = new QLineEdit(QStringLiteral("0"), cg); cl->addRow(QStringLiteral(" &Y:"), m_arcYEdit);
    arcRow->addWidget(cg, 1);
    auto* ag = new QGroupBox(QStringLiteral(" Arc Parameters   "), tab); ag->setMinimumWidth(150); auto* al = new QFormLayout(ag);
    m_arcRadiusEdit = new QLineEdit(QStringLiteral("0"), ag); al->addRow(QStringLiteral(" Ra&dius:"), m_arcRadiusEdit);
    m_arcStartAngleEdit = new QLineEdit(QStringLiteral("0"), ag); al->addRow(QStringLiteral(" &Start Angle:"), m_arcStartAngleEdit);
    m_arcEndAngleEdit = new QLineEdit(QStringLiteral("0"), ag); al->addRow(QStringLiteral(" &End Angle:"), m_arcEndAngleEdit);
    auto* arcContent = new QHBoxLayout();
    m_arcWFPanel = createWFPanel(tab, m_arcRefIndexEdit, m_arcFlipNCheck);
    arcContent->addWidget(m_arcWFPanel);
    arcContent->addLayout(arcRow, 1);
    l->addLayout(arcContent);
    auto* ar = new QHBoxLayout(); ar->addWidget(new QLabel(QStringLiteral(" &Number of control points:"), tab));
    m_arcAmntPtsEdit = new QLineEdit(QStringLiteral("0"), tab); ar->addWidget(m_arcAmntPtsEdit); l->addLayout(ar);
    auto* br = new QHBoxLayout(); auto* sb = new QPushButton(QStringLiteral("Save"), tab); sb->setDefault(true);
    auto* rb = new QPushButton(QStringLiteral("Restore"), tab); br->addStretch(); br->addWidget(rb); br->addWidget(sb); l->addStretch(); l->addLayout(br);
    m_tabs->addTab(wrapScroll(tab), QStringLiteral("Arc"));
    connect(sb, &QPushButton::clicked, this, &PropertiesWidget::onSaveArc);
    connect(rb, &QPushButton::clicked, this, &PropertiesWidget::onRestoreArc);
}

void PropertiesWidget::setupCurveTab() {
    auto* tab = new QWidget(); auto* l = new QVBoxLayout(tab);
    auto* nr = new QHBoxLayout(); auto* nl = new QLabel(QStringLiteral("Curve Name"), tab);
    QFont bf = nl->font(); bf.setBold(true); nl->setFont(bf); nr->addWidget(nl);
    m_cvWFStatusLabel = new QLabel(QStringLiteral("(Non WF)"), tab); QFont sf = m_cvWFStatusLabel->font(); sf.setBold(true); m_cvWFStatusLabel->setFont(sf);
    m_cvWFStatusLabel->setStyleSheet(QStringLiteral("color: navy;")); nr->addStretch(); nr->addWidget(m_cvWFStatusLabel); l->addLayout(nr);
    m_cvNameEdit = new QLineEdit(tab); l->addWidget(m_cvNameEdit);
    auto* pg = new QGroupBox(QStringLiteral(" List of Points Coordinates "), tab); auto* pl = new QVBoxLayout(pg);
    m_cvGrid = new QTableWidget(0, 2, pg); m_cvGrid->setHorizontalHeaderLabels({QStringLiteral("X"), QStringLiteral("Y")});
    m_cvGrid->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_cvGrid->setSelectionBehavior(QAbstractItemView::SelectRows); pl->addWidget(m_cvGrid); l->addWidget(pg);
    auto* br = new QHBoxLayout();
    m_cvAddBtn = new QPushButton(QStringLiteral("&Add"), tab); m_cvAddBtn->setVisible(false);
    m_cvDelBtn = new QPushButton(QStringLiteral("&Delete"), tab); m_cvDelBtn->setVisible(false);
    br->addWidget(m_cvAddBtn); br->addWidget(m_cvDelBtn); br->addStretch(); l->addLayout(br);
    m_cvEditCurveBtn = new QPushButton(QStringLiteral("Edit cur&ve"), tab); l->addWidget(m_cvEditCurveBtn);
    connect(m_cvEditCurveBtn, &QPushButton::clicked, this, [this]() {
        if (!m_currentObject) return;
        CurvePointsDialog dlg(this);
        dlg.setReadOnly(false);
        // Load current points into the dialog grid
        QTableWidget* dlgGrid = dlg.grid();
        dlgGrid->setRowCount(0);
        for (const auto& pt : m_currentObject->controlPoints()) {
            int r = dlgGrid->rowCount();
            dlgGrid->insertRow(r);
            dlgGrid->setItem(r, 0, new QTableWidgetItem(QString::number(pt.x(), 'f', 6)));
            dlgGrid->setItem(r, 1, new QTableWidgetItem(QString::number(pt.y(), 'f', 6)));
        }
        if (dlg.exec() == QDialog::Accepted) {
            // Write back to the curve grid (PropertiesWidget table)
            m_cvGrid->setRowCount(0);
            for (int r = 0; r < dlgGrid->rowCount(); ++r) {
                auto* xi = dlgGrid->item(r, 0);
                auto* yi = dlgGrid->item(r, 1);
                if (xi && yi) {
                    int nr = m_cvGrid->rowCount();
                    m_cvGrid->insertRow(nr);
                    m_cvGrid->setItem(nr, 0, new QTableWidgetItem(xi->text()));
                    m_cvGrid->setItem(nr, 1, new QTableWidgetItem(yi->text()));
                }
            }
            // Apply changes via onSaveCurve (which reads from m_cvGrid)
            onSaveCurve();
        }
    });
    auto* cvContent = new QHBoxLayout();
    m_cvWFPanel = createWFPanel(tab, m_cvRefIndexEdit, m_cvFlipNCheck);
    cvContent->addWidget(m_cvWFPanel);
    cvContent->addWidget(pg, 1);
    l->addLayout(cvContent);
    auto* br2 = new QHBoxLayout(); auto* sb = new QPushButton(QStringLiteral("Save"), tab); sb->setDefault(true);
    auto* rb = new QPushButton(QStringLiteral("Restore"), tab); br2->addStretch(); br2->addWidget(rb); br2->addWidget(sb); l->addStretch(); l->addLayout(br2);
    m_tabs->addTab(wrapScroll(tab), QStringLiteral("Curve"));
    connect(sb, &QPushButton::clicked, this, &PropertiesWidget::onSaveCurve);
    connect(rb, &QPushButton::clicked, this, &PropertiesWidget::onRestoreCurve);
}

void PropertiesWidget::setupObjectsTab() {
    auto* tab = new QWidget(); auto* l = new QVBoxLayout(tab);
    auto* nl = new QLabel(QStringLiteral("Objects Name"), tab); QFont bf = nl->font(); bf.setBold(true); nl->setFont(bf); l->addWidget(nl);
    m_objListWidget = new QListWidget(tab); l->addWidget(m_objListWidget);
    auto* br = new QHBoxLayout(); auto* ib = new QPushButton(QStringLiteral("&Insert"), tab);
    auto* db = new QPushButton(QStringLiteral("&Delete"), tab);
    auto* sel = new QPushButton(QStringLiteral("&Select"), tab); sel->setDefault(true);
    br->addWidget(ib); br->addWidget(db); br->addStretch(); br->addWidget(sel); l->addLayout(br);
    m_tabs->addTab(wrapScroll(tab), QStringLiteral("Objects"));
    connect(ib, &QPushButton::clicked, this, &PropertiesWidget::insertObjectRequested);
    connect(db, &QPushButton::clicked, this, &PropertiesWidget::deleteObjectRequested);
    connect(sel, &QPushButton::clicked, this, [this]{ if (m_objListWidget->currentItem()) emit selectObjectRequested(m_objListWidget->currentItem()->text()); });
}

void PropertiesWidget::setupCalcOvalTab() {
    auto* tab = new QWidget(); auto* l = new QVBoxLayout(tab);
    auto* tl = new QLabel(QStringLiteral("Cartesian Oval parameters"), tab); QFont bf = tl->font(); bf.setBold(true); tl->setFont(bf); l->addWidget(tl);
    m_cOvalNameEdit = new QLineEdit(tab); m_cOvalNameEdit->setReadOnly(true); l->addWidget(m_cOvalNameEdit);
    auto* f = new QFormLayout();
    m_cOvalWF1Edit = new QLineEdit(tab); m_cOvalWF1Edit->setReadOnly(true); f->addRow(QStringLiteral("First WF:"), m_cOvalWF1Edit);
    m_cOvalWF2Edit = new QLineEdit(tab); m_cOvalWF2Edit->setReadOnly(true); f->addRow(QStringLiteral("Second WF:"), m_cOvalWF2Edit);
    m_cOvalRefEdit = new QLineEdit(tab); m_cOvalRefEdit->setReadOnly(true); f->addRow(QStringLiteral("Ref. Point:"), m_cOvalRefEdit);
    m_cOvalQtyPtsEdit = new QLineEdit(tab); f->addRow(QStringLiteral("Calculations Qty:"), m_cOvalQtyPtsEdit); l->addLayout(f);
    auto* sep = new QFrame(tab); sep->setFrameShape(QFrame::HLine); sep->setFrameShadow(QFrame::Sunken); l->addWidget(sep);
    l->addWidget(new QLabel(QStringLiteral("Result object name:"), tab)); m_cOvalResultEdit = new QLineEdit(tab); m_cOvalResultEdit->setReadOnly(true); l->addWidget(m_cOvalResultEdit);
    auto* br = new QHBoxLayout(); auto* sb = new QPushButton(QStringLiteral("Save"), tab); sb->setDefault(true);
    auto* rb = new QPushButton(QStringLiteral("Restore"), tab); br->addStretch(); br->addWidget(rb); br->addWidget(sb); l->addStretch(); l->addLayout(br);
    m_tabs->addTab(wrapScroll(tab), QStringLiteral("CalcOval"));
    connect(sb, &QPushButton::clicked, this, &PropertiesWidget::onSaveCalcOval);
    connect(rb, &QPushButton::clicked, this, &PropertiesWidget::onRestoreCalcOval);
}

void PropertiesWidget::setupPropagateTab() {
    auto* tab = new QWidget(); auto* l = new QVBoxLayout(tab);
    auto* tl = new QLabel(QStringLiteral("WF Propagation parammeters"), tab); QFont bf = tl->font(); bf.setBold(true); tl->setFont(bf); l->addWidget(tl);
    m_propgNameEdit = new QLineEdit(tab); m_propgNameEdit->setReadOnly(true); l->addWidget(m_propgNameEdit);
    auto* f = new QFormLayout();
    m_propgWFEdit = new QLineEdit(tab); m_propgWFEdit->setReadOnly(true); f->addRow(QStringLiteral("Wave-Front"), m_propgWFEdit);
    m_propgIOREdit = new QLineEdit(tab); m_propgIOREdit->setReadOnly(true); f->addRow(QStringLiteral("Refract. Indx:"), m_propgIOREdit);
    m_propgSurfEdit = new QLineEdit(tab); m_propgSurfEdit->setReadOnly(true); f->addRow(QStringLiteral("Surface:"), m_propgSurfEdit);
    m_propgQtyPtsEdit = new QLineEdit(tab); f->addRow(QStringLiteral("Calculations Qty:"), m_propgQtyPtsEdit);
    m_propgOffsetEdit = new QLineEdit(tab); f->addRow(QStringLiteral("OPL Offset:"), m_propgOffsetEdit); l->addLayout(f);
    auto* sep = new QFrame(tab); sep->setFrameShape(QFrame::HLine); sep->setFrameShadow(QFrame::Sunken); l->addWidget(sep);
    l->addWidget(new QLabel(QStringLiteral("Result object name:"), tab)); m_propgResultEdit = new QLineEdit(tab); m_propgResultEdit->setReadOnly(true); l->addWidget(m_propgResultEdit);
    auto* br = new QHBoxLayout(); auto* sb = new QPushButton(QStringLiteral("Save"), tab); sb->setDefault(true);
    auto* rb = new QPushButton(QStringLiteral("Restore"), tab); br->addStretch(); br->addWidget(rb); br->addWidget(sb); l->addStretch(); l->addLayout(br);
    m_tabs->addTab(wrapScroll(tab), QStringLiteral("Propagate"));
    connect(sb, &QPushButton::clicked, this, &PropertiesWidget::onSavePropagate);
    connect(rb, &QPushButton::clicked, this, &PropertiesWidget::onRestorePropagate);
}

void PropertiesWidget::setObject(CustomObject* obj) { m_currentObject = obj; showObjectTabs(obj); }

void PropertiesWidget::refreshGridFromObject(CustomObject* obj)
{
    if (!obj || !m_cvGrid) return;
    // Only update if Curve tab is visible and this is the current object
    if (m_currentObject != obj) return;
    m_cvGrid->setRowCount(0);
    for (const auto& pt : obj->controlPoints()) {
        int r = m_cvGrid->rowCount();
        m_cvGrid->insertRow(r);
        m_cvGrid->setItem(r, 0, new QTableWidgetItem(QString::number(pt.x(), 'f', 6)));
        m_cvGrid->setItem(r, 1, new QTableWidgetItem(QString::number(pt.y(), 'f', 6)));
    }
}

void PropertiesWidget::setOperation(CustomOperation* op)
{
    m_currentObject = nullptr;
    m_currentOperation = op;
    if (!op) { m_tabs->setCurrentIndex(0); return; }
    switch (op->operationType()) {
    case OperationType::CartesianOval:
        m_cOvalNameEdit->setText(op->name());
        m_cOvalNameEdit->setReadOnly(false); // editable
        m_cOvalQtyPtsEdit->setText(QString::number(op->amountOfPoints()));
        m_cOvalQtyPtsEdit->setReadOnly(false);
        m_cOvalWF1Edit->setText(op->paramName(0));
        m_cOvalWF2Edit->setText(op->paramName(1));
        m_cOvalRefEdit->setText(op->paramName(2));
        m_cOvalResultEdit->setText(op->resultName());
        m_cOvalResultEdit->setReadOnly(false);
        m_tabs->setCurrentIndex(6);
        break;
    case OperationType::PropagateWF:
        m_propgNameEdit->setText(op->name());
        m_propgNameEdit->setReadOnly(false); // editable
        m_propgQtyPtsEdit->setText(QString::number(op->amountOfPoints()));
        m_propgQtyPtsEdit->setReadOnly(false);
        m_propgWFEdit->setText(op->paramName(0));
        m_propgSurfEdit->setText(op->paramName(1));
        m_propgIOREdit->setText(op->paramName(2));
        {
            auto* pop = dynamic_cast<PropagateWFOperation*>(op);
            m_propgOffsetEdit->setText(QString::number(pop ? pop->offset() : 0.0, 'f', 6));
            m_propgOffsetEdit->setReadOnly(false);
        }
        m_propgResultEdit->setText(op->resultName());
        m_propgResultEdit->setReadOnly(false);
        m_tabs->setCurrentIndex(7);
        break;
    default:
        m_tabs->setCurrentIndex(0);
        break;
    }
}

void PropertiesWidget::setProject(Project* project) {
    m_currentProject = project;
    if (project) {
        m_prjNameEdit->setText(project->name());
        m_prjXAutoCheck->setEnabled(true); m_prjYAutoCheck->setEnabled(true);
        m_objListWidget->clear();
        for (auto* o : project->dataObjects()) if (o) m_objListWidget->addItem(o->name());
        for (auto* o : project->resultObjects()) if (o) m_objListWidget->addItem(o->name());
    }
    m_tabs->setCurrentIndex(0);
}

void PropertiesWidget::setOperations(Project*) {}
void PropertiesWidget::refresh() { if (m_currentProject) setProject(m_currentProject); if (m_currentObject) showObjectTabs(m_currentObject); }

void PropertiesWidget::showObjectTabs(CustomObject* obj) {
    if (!obj) { m_tabs->setCurrentIndex(0); return; }
    auto updateWFLabel = [](QLabel* l, ObjectType t) {
        if (isWavefront(t)) { l->setText(isVirtualWF(t) ? QStringLiteral("(WF Virtual)") : QStringLiteral("(WF)")); l->setStyleSheet(QStringLiteral("color: maroon; font-weight: bold;")); }
        else { l->setText(QStringLiteral("(Non WF)")); l->setStyleSheet(QStringLiteral("color: navy; font-weight: bold;")); }
    };
    switch (toBaseType(obj->objectType())) {
    case 0x001: m_ptNameEdit->setText(obj->name()); m_ptRefIndexEdit->setText(QString::number(obj->refractiveIndex())); updateWFLabel(m_ptWFStatusLabel, obj->objectType());
        m_ptWFPanel->setVisible(obj->isWavefront());
        if (!obj->controlPoints().isEmpty()) { m_ptXEdit->setText(QString::number(obj->controlPoints().first().x(), 'f', 6)); m_ptYEdit->setText(QString::number(obj->controlPoints().first().y(), 'f', 6)); }
        m_ptFlipNCheck->setChecked(obj->isNormalFlipped());
        m_tabs->setCurrentIndex(1); break;
    case 0x002: m_lnNameEdit->setText(obj->name()); m_lnRefIndexEdit->setText(QString::number(obj->refractiveIndex())); updateWFLabel(m_lnWFStatusLabel, obj->objectType());
        m_lnWFPanel->setVisible(obj->isWavefront());
        if (obj->controlPoints().size() >= 2) { m_lnP1XEdit->setText(QString::number(obj->controlPoints()[0].x(), 'f', 6)); m_lnP1YEdit->setText(QString::number(obj->controlPoints()[0].y(), 'f', 6)); m_lnP2XEdit->setText(QString::number(obj->controlPoints()[1].x(), 'f', 6)); m_lnP2YEdit->setText(QString::number(obj->controlPoints()[1].y(), 'f', 6)); }
        m_lnFlipNCheck->setChecked(obj->isNormalFlipped());
        m_tabs->setCurrentIndex(2); break;
    case 0x003: m_arcNameEdit->setText(obj->name()); m_arcRefIndexEdit->setText(QString::number(obj->refractiveIndex())); updateWFLabel(m_arcWFStatusLabel, obj->objectType());
        m_arcWFPanel->setVisible(obj->isWavefront());
        if (auto* ao = dynamic_cast<ArcObject*>(obj)) { m_arcXEdit->setText(QString::number(ao->center().x(), 'f', 6)); m_arcYEdit->setText(QString::number(ao->center().y(), 'f', 6)); }
        if (auto* ao = dynamic_cast<ArcObject*>(obj)) { m_arcRadiusEdit->setText(QString::number(ao->radius())); m_arcStartAngleEdit->setText(QString::number(ao->startAngle())); m_arcEndAngleEdit->setText(QString::number(ao->endAngle())); m_arcAmntPtsEdit->setText(QString::number(ao->numPoints())); }
        m_arcFlipNCheck->setChecked(obj->isNormalFlipped());
        m_tabs->setCurrentIndex(3); break;
    default: m_cvNameEdit->setText(obj->name()); m_cvRefIndexEdit->setText(QString::number(obj->refractiveIndex())); updateWFLabel(m_cvWFStatusLabel, obj->objectType());
        m_cvWFPanel->setVisible(obj->isWavefront());
        m_cvGrid->setRowCount(0);
        for (const auto& pt : obj->controlPoints()) { int r = m_cvGrid->rowCount(); m_cvGrid->insertRow(r); m_cvGrid->setItem(r, 0, new QTableWidgetItem(QString::number(pt.x(), 'f', 6))); m_cvGrid->setItem(r, 1, new QTableWidgetItem(QString::number(pt.y(), 'f', 6))); }
        m_cvFlipNCheck->setChecked(obj->isNormalFlipped());
        m_tabs->setCurrentIndex(4); break;
    }
}

// ========== SAVE SLOTS - read from UI fields and write to object ==========
void PropertiesWidget::onSaveProject() {
    if (m_currentProject) { m_currentProject->setName(m_prjNameEdit->text()); emit projectModified(m_currentProject); }
}
void PropertiesWidget::onRestoreProject() { if (m_currentProject) { m_prjNameEdit->setText(m_currentProject->name()); } }

void PropertiesWidget::onSavePoint() {
    if (!m_currentObject) return;
    QString oldName = m_currentObject->name();
    double oldIR = m_currentObject->refractiveIndex();
    bool oldFlip = m_currentObject->isNormalFlipped();
    QVector<QPointF> oldPts = m_currentObject->controlPoints();
    QString newName = m_ptNameEdit->text();
    double newIR = m_ptRefIndexEdit->text().toDouble();
    bool newFlip = m_ptFlipNCheck->isChecked();
    QPointF pt(m_ptXEdit->text().toDouble(), m_ptYEdit->text().toDouble());
    QVector<QPointF> newPts = {pt};
    if (m_cmdHistory) {
        auto cmd = std::make_unique<ModifyObjectPropertiesCommand>(
            m_currentObject, oldName, oldIR, oldFlip, oldPts, newName, newIR, newFlip, newPts);
        m_cmdHistory->push(std::move(cmd), m_currentProject);
    }
    emit objectModified(m_currentObject);
}
void PropertiesWidget::onRestorePoint() { if (m_currentObject) showObjectTabs(m_currentObject); }

void PropertiesWidget::onSaveLine() {
    if (!m_currentObject) return;
    QString oldName = m_currentObject->name();
    double oldIR = m_currentObject->refractiveIndex();
    bool oldFlip = m_currentObject->isNormalFlipped();
    QVector<QPointF> oldPts = m_currentObject->controlPoints();
    QString newName = m_lnNameEdit->text();
    double newIR = m_lnRefIndexEdit->text().toDouble();
    bool newFlip = m_lnFlipNCheck->isChecked();
    QPointF p1(m_lnP1XEdit->text().toDouble(), m_lnP1YEdit->text().toDouble());
    QPointF p2(m_lnP2XEdit->text().toDouble(), m_lnP2YEdit->text().toDouble());
    QVector<QPointF> newPts = {p1, p2};
    if (m_cmdHistory) {
        auto cmd = std::make_unique<ModifyObjectPropertiesCommand>(
            m_currentObject, oldName, oldIR, oldFlip, oldPts, newName, newIR, newFlip, newPts);
        m_cmdHistory->push(std::move(cmd), m_currentProject);
    }
    emit objectModified(m_currentObject);
}
void PropertiesWidget::onRestoreLine() { if (m_currentObject) showObjectTabs(m_currentObject); }

void PropertiesWidget::onSaveArc() {
    if (!m_currentObject) return;
    QString oldName = m_currentObject->name();
    double oldIR = m_currentObject->refractiveIndex();
    bool oldFlip = m_currentObject->isNormalFlipped();
    QVector<QPointF> oldPts = m_currentObject->controlPoints();
    m_currentObject->setName(m_arcNameEdit->text());
    m_currentObject->setRefractiveIndex(m_arcRefIndexEdit->text().toDouble());
    m_currentObject->setNormalFlipped(m_arcFlipNCheck->isChecked());
    auto* ao = dynamic_cast<ArcObject*>(m_currentObject);
    if (ao) {
        ao->setCenter(QPointF(m_arcXEdit->text().toDouble(), m_arcYEdit->text().toDouble()));
        ao->setRadius(m_arcRadiusEdit->text().toDouble());
        ao->setStartAngle(m_arcStartAngleEdit->text().toDouble());
        ao->setEndAngle(m_arcEndAngleEdit->text().toDouble());
        ao->setNumPoints(m_arcAmntPtsEdit->text().toInt());
        ao->setControlPoints(ao->generateArcPoints());
    }
    QVector<QPointF> newPts = m_currentObject->controlPoints();
    if (m_cmdHistory) {
        auto cmd = std::make_unique<ModifyObjectPropertiesCommand>(
            m_currentObject, oldName, oldIR, oldFlip, oldPts, m_currentObject->name(), m_currentObject->refractiveIndex(), m_currentObject->isNormalFlipped(), newPts);
        m_cmdHistory->push(std::move(cmd), m_currentProject);
    }
    emit objectModified(m_currentObject);
}
void PropertiesWidget::onRestoreArc() { if (m_currentObject) showObjectTabs(m_currentObject); }

void PropertiesWidget::onSaveCurve() {
    if (!m_currentObject) return;
    QString oldName = m_currentObject->name();
    double oldIR = m_currentObject->refractiveIndex();
    bool oldFlip = m_currentObject->isNormalFlipped();
    QVector<QPointF> oldPts = m_currentObject->controlPoints();
    QString newName = m_cvNameEdit->text();
    double newIR = m_cvRefIndexEdit->text().toDouble();
    bool newFlip = m_cvFlipNCheck->isChecked();
    QVector<QPointF> newPts;
    for (int r = 0; r < m_cvGrid->rowCount(); ++r) {
        auto* xi = m_cvGrid->item(r, 0); auto* yi = m_cvGrid->item(r, 1);
        if (xi && yi) newPts.append(QPointF(xi->text().toDouble(), yi->text().toDouble()));
    }
    if (m_cmdHistory) {
        auto cmd = std::make_unique<ModifyObjectPropertiesCommand>(
            m_currentObject, oldName, oldIR, oldFlip, oldPts, newName, newIR, newFlip, newPts);
        m_cmdHistory->push(std::move(cmd), m_currentProject);
    }
    emit objectModified(m_currentObject);
}
void PropertiesWidget::onRestoreCurve() { if (m_currentObject) showObjectTabs(m_currentObject); }

void PropertiesWidget::onSaveCalcOval()
{
    if (!m_currentOperation) return;
    // Update modifiable (non-object-reference) parameters from UI
    m_currentOperation->setName(m_cOvalNameEdit->text());
    m_currentOperation->setAmountOfPoints(m_cOvalQtyPtsEdit->text().toInt());
    m_currentOperation->setParamName(0, m_cOvalWF1Edit->text());
    m_currentOperation->setParamName(1, m_cOvalWF2Edit->text());
    m_currentOperation->setParamName(2, m_cOvalRefEdit->text());
    emit operationModified(m_currentOperation);
}
void PropertiesWidget::onRestoreCalcOval()
{
    if (m_currentOperation && m_currentOperation->operationType() == OperationType::CartesianOval)
        setOperation(m_currentOperation);
}
void PropertiesWidget::onSavePropagate()
{
    if (!m_currentOperation) return;
    m_currentOperation->setName(m_propgNameEdit->text());
    m_currentOperation->setAmountOfPoints(m_propgQtyPtsEdit->text().toInt());
    m_currentOperation->setParamName(0, m_propgWFEdit->text());
    m_currentOperation->setParamName(1, m_propgSurfEdit->text());
    m_currentOperation->setParamName(2, m_propgIOREdit->text());
    if (auto* pop = dynamic_cast<PropagateWFOperation*>(m_currentOperation))
        pop->setOffset(m_propgOffsetEdit->text().toDouble());
    emit operationModified(m_currentOperation);
}
void PropertiesWidget::onRestorePropagate()
{
    if (m_currentOperation && m_currentOperation->operationType() == OperationType::PropagateWF)
        setOperation(m_currentOperation);
}

} // namespace ExpressDesigner