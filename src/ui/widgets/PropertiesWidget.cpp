#include "PropertiesWidget.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFrame>
#include <QHeaderView>
#include <QMessageBox>
#include <core/ObjectTypes.h>
#include <core/ArcObject.h>
#include <core/PointObject.h>
#include <core/LineObject.h>
#include <core/CurveObject.h>

namespace ExpressDesigner {

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
}

void PropertiesWidget::setupProjectTab() {
    auto* tab = new QWidget(); auto* l = new QVBoxLayout(tab);
    auto* nr = new QHBoxLayout(); auto* nl = new QLabel(QStringLiteral("Project Name"), tab);
    QFont bf = nl->font(); bf.setBold(true); nl->setFont(bf); nr->addWidget(nl); l->addLayout(nr);
    m_prjNameEdit = new QLineEdit(tab); l->addWidget(m_prjNameEdit);
    auto* xg = new QGroupBox(QStringLiteral(" X Axis Scale "), tab); auto* xl = new QFormLayout(xg);
    m_prjXMinEdit = new QLineEdit(QStringLiteral("0"), xg); m_prjXMinEdit->setEnabled(false); xl->addRow(QStringLiteral("&Min:"), m_prjXMinEdit);
    m_prjXMaxEdit = new QLineEdit(QStringLiteral("0"), xg); m_prjXMaxEdit->setEnabled(false); xl->addRow(QStringLiteral("M&ax:"), m_prjXMaxEdit);
    l->addWidget(xg); m_prjXAutoCheck = new QCheckBox(QStringLiteral("Auto scale X axis"), tab); m_prjXAutoCheck->setEnabled(false); l->addWidget(m_prjXAutoCheck);
    auto* yg = new QGroupBox(QStringLiteral(" Y Axis Scale "), tab); auto* yl = new QFormLayout(yg);
    m_prjYMinEdit = new QLineEdit(QStringLiteral("0"), yg); m_prjYMinEdit->setEnabled(false); yl->addRow(QStringLiteral("&Min:"), m_prjYMinEdit);
    m_prjYMaxEdit = new QLineEdit(QStringLiteral("0"), yg); m_prjYMaxEdit->setEnabled(false); yl->addRow(QStringLiteral("M&ax:"), m_prjYMaxEdit);
    l->addWidget(yg); m_prjYAutoCheck = new QCheckBox(QStringLiteral("Auto scale Y axis"), tab); m_prjYAutoCheck->setEnabled(false); l->addWidget(m_prjYAutoCheck);
    auto* br = new QHBoxLayout(); auto* sb = new QPushButton(QStringLiteral("Save"), tab); sb->setDefault(true);
    auto* rb = new QPushButton(QStringLiteral("Restore"), tab); br->addStretch(); br->addWidget(rb); br->addWidget(sb); l->addLayout(br); l->addStretch();
    m_tabs->addTab(tab, QStringLiteral("Project"));
    connect(sb, &QPushButton::clicked, this, &PropertiesWidget::onSaveProject);
    connect(rb, &QPushButton::clicked, this, &PropertiesWidget::onRestoreProject);
}

void PropertiesWidget::setupPointTab() {
    auto* tab = new QWidget(); auto* l = new QVBoxLayout(tab);
    auto* nr = new QHBoxLayout(); auto* nl = new QLabel(QStringLiteral("Point Name"), tab);
    QFont bf = nl->font(); bf.setBold(true); nl->setFont(bf); nr->addWidget(nl);
    m_ptWFStatusLabel = new QLabel(QStringLiteral("(Non WF)"), tab); QFont sf = m_ptWFStatusLabel->font(); sf.setBold(true); m_ptWFStatusLabel->setFont(sf);
    m_ptWFStatusLabel->setStyleSheet(QStringLiteral("color: navy;")); nr->addStretch(); nr->addWidget(m_ptWFStatusLabel); l->addLayout(nr);
    m_ptNameEdit = new QLineEdit(tab); l->addWidget(m_ptNameEdit);
    auto* rr = new QHBoxLayout(); rr->addWidget(new QLabel(QStringLiteral("&Refractive Index:"), tab));
    m_ptRefIndexEdit = new QLineEdit(QStringLiteral("0"), tab); rr->addWidget(m_ptRefIndexEdit); l->addLayout(rr);
    auto* pg = new QGroupBox(QStringLiteral(" Point Coordinates "), tab); auto* pl = new QFormLayout(pg);
    m_ptXEdit = new QLineEdit(QStringLiteral("0"), pg); pl->addRow(QStringLiteral(" &X:"), m_ptXEdit);
    m_ptYEdit = new QLineEdit(QStringLiteral("0"), pg); pl->addRow(QStringLiteral(" &Y:"), m_ptYEdit); l->addWidget(pg);
    m_ptFlipNCheck = new QCheckBox(QStringLiteral("Flip normals"), tab); l->addWidget(m_ptFlipNCheck);
    auto* br = new QHBoxLayout(); auto* sb = new QPushButton(QStringLiteral("Save"), tab); sb->setDefault(true);
    auto* rb = new QPushButton(QStringLiteral("Restore"), tab); br->addStretch(); br->addWidget(rb); br->addWidget(sb); l->addLayout(br); l->addStretch();
    m_tabs->addTab(tab, QStringLiteral("Point"));
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
    auto* rr = new QHBoxLayout(); rr->addWidget(new QLabel(QStringLiteral("&Refractive Index:"), tab));
    m_lnRefIndexEdit = new QLineEdit(QStringLiteral("0"), tab); rr->addWidget(m_lnRefIndexEdit); l->addLayout(rr);
    auto* p1g = new QGroupBox(QStringLiteral(" First Point Coordinates "), tab); auto* p1l = new QFormLayout(p1g);
    m_lnP1XEdit = new QLineEdit(QStringLiteral("0"), p1g); p1l->addRow(QStringLiteral(" &X:"), m_lnP1XEdit);
    m_lnP1YEdit = new QLineEdit(QStringLiteral("0"), p1g); p1l->addRow(QStringLiteral(" &Y:"), m_lnP1YEdit); l->addWidget(p1g);
    auto* p2g = new QGroupBox(QStringLiteral(" Second Point Coordinates "), tab); auto* p2l = new QFormLayout(p2g);
    m_lnP2XEdit = new QLineEdit(QStringLiteral("0"), p2g); p2l->addRow(QStringLiteral(" &X:"), m_lnP2XEdit);
    m_lnP2YEdit = new QLineEdit(QStringLiteral("0"), p2g); p2l->addRow(QStringLiteral(" &Y:"), m_lnP2YEdit); l->addWidget(p2g);
    m_lnFlipNCheck = new QCheckBox(QStringLiteral("Flip normals"), tab); l->addWidget(m_lnFlipNCheck);
    auto* br = new QHBoxLayout(); auto* sb = new QPushButton(QStringLiteral("Save"), tab); sb->setDefault(true);
    auto* rb = new QPushButton(QStringLiteral("Restore"), tab); br->addStretch(); br->addWidget(rb); br->addWidget(sb); l->addLayout(br); l->addStretch();
    m_tabs->addTab(tab, QStringLiteral("Line"));
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
    auto* rr = new QHBoxLayout(); rr->addWidget(new QLabel(QStringLiteral("&Refractive Index:"), tab));
    m_arcRefIndexEdit = new QLineEdit(QStringLiteral("0"), tab); rr->addWidget(m_arcRefIndexEdit); l->addLayout(rr);
    auto* cg = new QGroupBox(QStringLiteral(" Center Point Coordinates "), tab); auto* cl = new QFormLayout(cg);
    m_arcXEdit = new QLineEdit(QStringLiteral("0"), cg); cl->addRow(QStringLiteral(" &X:"), m_arcXEdit);
    m_arcYEdit = new QLineEdit(QStringLiteral("0"), cg); cl->addRow(QStringLiteral(" &Y:"), m_arcYEdit); l->addWidget(cg);
    auto* ag = new QGroupBox(QStringLiteral(" Arc Parameters   "), tab); auto* al = new QFormLayout(ag);
    m_arcRadiusEdit = new QLineEdit(QStringLiteral("0"), ag); al->addRow(QStringLiteral(" Ra&dius:"), m_arcRadiusEdit);
    m_arcStartAngleEdit = new QLineEdit(QStringLiteral("0"), ag); al->addRow(QStringLiteral(" &Start Angle:"), m_arcStartAngleEdit);
    m_arcEndAngleEdit = new QLineEdit(QStringLiteral("0"), ag); al->addRow(QStringLiteral(" &End Angle:"), m_arcEndAngleEdit); l->addWidget(ag);
    auto* ar = new QHBoxLayout(); ar->addWidget(new QLabel(QStringLiteral(" &Number of control points:"), tab));
    m_arcAmntPtsEdit = new QLineEdit(QStringLiteral("0"), tab); ar->addWidget(m_arcAmntPtsEdit); l->addLayout(ar);
    m_arcFlipNCheck = new QCheckBox(QStringLiteral("Flip normals"), tab); l->addWidget(m_arcFlipNCheck);
    auto* br = new QHBoxLayout(); auto* sb = new QPushButton(QStringLiteral("Save"), tab); sb->setDefault(true);
    auto* rb = new QPushButton(QStringLiteral("Restore"), tab); br->addStretch(); br->addWidget(rb); br->addWidget(sb); l->addLayout(br); l->addStretch();
    m_tabs->addTab(tab, QStringLiteral("Arc"));
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
    auto* rr = new QHBoxLayout(); rr->addWidget(new QLabel(QStringLiteral("&Refractive Index:"), tab));
    m_cvRefIndexEdit = new QLineEdit(QStringLiteral("0"), tab); rr->addWidget(m_cvRefIndexEdit); l->addLayout(rr);
    auto* pg = new QGroupBox(QStringLiteral(" List of Points Coordinates "), tab); auto* pl = new QVBoxLayout(pg);
    auto* hr = new QHBoxLayout(); hr->addStretch(3); hr->addWidget(new QLabel(QStringLiteral(" X"), pg)); hr->addStretch(1); hr->addWidget(new QLabel(QStringLiteral(" Y"), pg)); hr->addStretch(3); pl->addLayout(hr);
    m_cvGrid = new QTableWidget(0, 2, pg); m_cvGrid->setHorizontalHeaderLabels({QStringLiteral("X"), QStringLiteral("Y")}); m_cvGrid->horizontalHeader()->setStretchLastSection(true); m_cvGrid->setSelectionBehavior(QAbstractItemView::SelectRows); pl->addWidget(m_cvGrid); l->addWidget(pg);
    auto* br = new QHBoxLayout();
    m_cvAddBtn = new QPushButton(QStringLiteral("&Add"), tab); m_cvAddBtn->setVisible(false);
    m_cvDelBtn = new QPushButton(QStringLiteral("&Delete"), tab); m_cvDelBtn->setVisible(false);
    br->addWidget(m_cvAddBtn); br->addWidget(m_cvDelBtn); br->addStretch(); l->addLayout(br);
    m_cvEditCurveBtn = new QPushButton(QStringLiteral("Edit cur&ve"), tab); l->addWidget(m_cvEditCurveBtn);
    m_cvFlipNCheck = new QCheckBox(QStringLiteral("Flip normals"), tab); l->addWidget(m_cvFlipNCheck);
    auto* br2 = new QHBoxLayout(); auto* sb = new QPushButton(QStringLiteral("Save"), tab); sb->setDefault(true);
    auto* rb = new QPushButton(QStringLiteral("Restore"), tab); br2->addStretch(); br2->addWidget(rb); br2->addWidget(sb); l->addLayout(br2); l->addStretch();
    m_tabs->addTab(tab, QStringLiteral("Curve"));
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
    m_tabs->addTab(tab, QStringLiteral("Objects"));
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
    auto* rb = new QPushButton(QStringLiteral("Restore"), tab); br->addStretch(); br->addWidget(rb); br->addWidget(sb); l->addLayout(br); l->addStretch();
    m_tabs->addTab(tab, QStringLiteral("CalcOval"));
    connect(sb, &QPushButton::clicked, this, &PropertiesWidget::calculateOvalRequested);
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
    auto* rb = new QPushButton(QStringLiteral("Restore"), tab); br->addStretch(); br->addWidget(rb); br->addWidget(sb); l->addLayout(br); l->addStretch();
    m_tabs->addTab(tab, QStringLiteral("Propagate"));
    connect(sb, &QPushButton::clicked, this, &PropertiesWidget::propagateWFRequested);
    connect(rb, &QPushButton::clicked, this, &PropertiesWidget::onRestorePropagate);
}

void PropertiesWidget::setObject(CustomObject* obj) { m_currentObject = obj; showObjectTabs(obj); }

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
        if (!obj->controlPoints().isEmpty()) { m_ptXEdit->setText(QString::number(obj->controlPoints().first().x(), 'f', 6)); m_ptYEdit->setText(QString::number(obj->controlPoints().first().y(), 'f', 6)); }
        m_ptFlipNCheck->setChecked(obj->isNormalFlipped()); m_ptFlipNCheck->setVisible(obj->isWavefront());
        m_tabs->setCurrentIndex(1); break;
    case 0x002: m_lnNameEdit->setText(obj->name()); m_lnRefIndexEdit->setText(QString::number(obj->refractiveIndex())); updateWFLabel(m_lnWFStatusLabel, obj->objectType());
        if (obj->controlPoints().size() >= 2) { m_lnP1XEdit->setText(QString::number(obj->controlPoints()[0].x(), 'f', 6)); m_lnP1YEdit->setText(QString::number(obj->controlPoints()[0].y(), 'f', 6)); m_lnP2XEdit->setText(QString::number(obj->controlPoints()[1].x(), 'f', 6)); m_lnP2YEdit->setText(QString::number(obj->controlPoints()[1].y(), 'f', 6)); }
        m_lnFlipNCheck->setChecked(obj->isNormalFlipped()); m_lnFlipNCheck->setVisible(obj->isWavefront());
        m_tabs->setCurrentIndex(2); break;
    case 0x003: m_arcNameEdit->setText(obj->name()); m_arcRefIndexEdit->setText(QString::number(obj->refractiveIndex())); updateWFLabel(m_arcWFStatusLabel, obj->objectType());
        if (auto* ao = dynamic_cast<ArcObject*>(obj)) { m_arcXEdit->setText(QString::number(ao->center().x(), 'f', 6)); m_arcYEdit->setText(QString::number(ao->center().y(), 'f', 6)); }
        if (auto* ao = dynamic_cast<ArcObject*>(obj)) { m_arcRadiusEdit->setText(QString::number(ao->radius())); m_arcStartAngleEdit->setText(QString::number(ao->startAngle())); m_arcEndAngleEdit->setText(QString::number(ao->endAngle())); m_arcAmntPtsEdit->setText(QString::number(ao->numPoints())); }
        m_arcFlipNCheck->setChecked(obj->isNormalFlipped()); m_arcFlipNCheck->setVisible(obj->isWavefront());
        m_tabs->setCurrentIndex(3); break;
    default: m_cvNameEdit->setText(obj->name()); m_cvRefIndexEdit->setText(QString::number(obj->refractiveIndex())); updateWFLabel(m_cvWFStatusLabel, obj->objectType());
        m_cvGrid->setRowCount(0);
        for (const auto& pt : obj->controlPoints()) { int r = m_cvGrid->rowCount(); m_cvGrid->insertRow(r); m_cvGrid->setItem(r, 0, new QTableWidgetItem(QString::number(pt.x(), 'f', 6))); m_cvGrid->setItem(r, 1, new QTableWidgetItem(QString::number(pt.y(), 'f', 6))); }
        m_cvFlipNCheck->setChecked(obj->isNormalFlipped()); m_cvFlipNCheck->setVisible(obj->isWavefront());
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
    m_currentObject->setName(m_ptNameEdit->text());
    m_currentObject->setRefractiveIndex(m_ptRefIndexEdit->text().toDouble());
    m_currentObject->setNormalFlipped(m_ptFlipNCheck->isChecked());
    QPointF pt(m_ptXEdit->text().toDouble(), m_ptYEdit->text().toDouble());
    if (m_currentObject->controlPoints().isEmpty())
        m_currentObject->addControlPoint(pt);
    else
        m_currentObject->updateControlPoint(0, pt);
    emit objectModified(m_currentObject);
}
void PropertiesWidget::onRestorePoint() { if (m_currentObject) showObjectTabs(m_currentObject); }

void PropertiesWidget::onSaveLine() {
    if (!m_currentObject) return;
    m_currentObject->setName(m_lnNameEdit->text());
    m_currentObject->setRefractiveIndex(m_lnRefIndexEdit->text().toDouble());
    m_currentObject->setNormalFlipped(m_lnFlipNCheck->isChecked());
    QPointF p1(m_lnP1XEdit->text().toDouble(), m_lnP1YEdit->text().toDouble());
    QPointF p2(m_lnP2XEdit->text().toDouble(), m_lnP2YEdit->text().toDouble());
    auto* ln = dynamic_cast<LineObject*>(m_currentObject);
    if (ln) { ln->setStartPoint(p1); ln->setEndPoint(p2); }
    else { if (m_currentObject->controlPoints().size() < 2) { m_currentObject->clearControlPoints(); m_currentObject->addControlPoint(p1); m_currentObject->addControlPoint(p2); } else { m_currentObject->updateControlPoint(0, p1); m_currentObject->updateControlPoint(1, p2); } }
    emit objectModified(m_currentObject);
}
void PropertiesWidget::onRestoreLine() { if (m_currentObject) showObjectTabs(m_currentObject); }

void PropertiesWidget::onSaveArc() {
    if (!m_currentObject) return;
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
        // Generate control points from arc params
        ao->setControlPoints(ao->generateArcPoints());
    }
    emit objectModified(m_currentObject);
}
void PropertiesWidget::onRestoreArc() { if (m_currentObject) showObjectTabs(m_currentObject); }

void PropertiesWidget::onSaveCurve() {
    if (!m_currentObject) return;
    m_currentObject->setName(m_cvNameEdit->text());
    m_currentObject->setRefractiveIndex(m_cvRefIndexEdit->text().toDouble());
    m_currentObject->setNormalFlipped(m_cvFlipNCheck->isChecked());
    QVector<QPointF> pts;
    for (int r = 0; r < m_cvGrid->rowCount(); ++r) {
        auto* xi = m_cvGrid->item(r, 0); auto* yi = m_cvGrid->item(r, 1);
        if (xi && yi) pts.append(QPointF(xi->text().toDouble(), yi->text().toDouble()));
    }
    m_currentObject->setControlPoints(pts);
    emit objectModified(m_currentObject);
}
void PropertiesWidget::onRestoreCurve() { if (m_currentObject) showObjectTabs(m_currentObject); }

void PropertiesWidget::onSaveCalcOval() { emit calculateOvalRequested(); }
void PropertiesWidget::onRestoreCalcOval() {}
void PropertiesWidget::onSavePropagate() { emit propagateWFRequested(); }
void PropertiesWidget::onRestorePropagate() {}

} // namespace ExpressDesigner