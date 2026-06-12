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

namespace ExpressDesigner {

PropertiesWidget::PropertiesWidget(QWidget* parent) : QWidget(parent)
{
    m_tabs = new QTabWidget(this);
    m_tabs->setTabPosition(QTabWidget::South);
    m_tabs->setStyleSheet(QStringLiteral("QTabWidget::pane { border: 1px solid #C4C4C3; }"));

    setupProjectTab();
    setupPointTab();
    setupLineTab();
    setupArcTab();
    setupCurveTab();
    setupObjectsTab();
    setupCalcOvalTab();
    setupPropagateTab();

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_tabs);
    m_tabs->setCurrentIndex(0);
}

// ========== PROJECT TAB ==========
void PropertiesWidget::setupProjectTab()
{
    auto* tab = new QWidget();
    auto* layout = new QVBoxLayout(tab);

    auto* nameRow = new QHBoxLayout();
    auto* nameLabel = new QLabel(QStringLiteral("Project Name"), tab);
    QFont boldFont = nameLabel->font(); boldFont.setBold(true); nameLabel->setFont(boldFont);
    nameRow->addWidget(nameLabel); layout->addLayout(nameRow);

    m_prjNameEdit = new QLineEdit(tab); layout->addWidget(m_prjNameEdit);

    auto* xGroup = new QGroupBox(QStringLiteral(" X Axis Scale "), tab);
    auto* xLayout = new QFormLayout(xGroup);
    m_prjXMinEdit = new QLineEdit(QStringLiteral("0"), xGroup); m_prjXMinEdit->setEnabled(false);
    xLayout->addRow(QStringLiteral("&Min:"), m_prjXMinEdit);
    m_prjXMaxEdit = new QLineEdit(QStringLiteral("0"), xGroup); m_prjXMaxEdit->setEnabled(false);
    xLayout->addRow(QStringLiteral("M&ax:"), m_prjXMaxEdit);
    layout->addWidget(xGroup);
    m_prjXAutoCheck = new QCheckBox(QStringLiteral("Auto scale X axis"), tab); m_prjXAutoCheck->setEnabled(false);
    layout->addWidget(m_prjXAutoCheck);

    auto* yGroup = new QGroupBox(QStringLiteral(" Y Axis Scale "), tab);
    auto* yLayout = new QFormLayout(yGroup);
    m_prjYMinEdit = new QLineEdit(QStringLiteral("0"), yGroup); m_prjYMinEdit->setEnabled(false);
    yLayout->addRow(QStringLiteral("&Min:"), m_prjYMinEdit);
    m_prjYMaxEdit = new QLineEdit(QStringLiteral("0"), yGroup); m_prjYMaxEdit->setEnabled(false);
    yLayout->addRow(QStringLiteral("M&ax:"), m_prjYMaxEdit);
    layout->addWidget(yGroup);
    m_prjYAutoCheck = new QCheckBox(QStringLiteral("Auto scale Y axis"), tab); m_prjYAutoCheck->setEnabled(false);
    layout->addWidget(m_prjYAutoCheck);

    auto* btnRow = new QHBoxLayout();
    auto* saveBtn = new QPushButton(QStringLiteral("Save"), tab); saveBtn->setDefault(true);
    auto* restoreBtn = new QPushButton(QStringLiteral("Restore"), tab);
    btnRow->addStretch(); btnRow->addWidget(restoreBtn); btnRow->addWidget(saveBtn);
    layout->addLayout(btnRow);
    layout->addStretch();
    m_tabs->addTab(tab, QStringLiteral("Project"));
    connect(saveBtn, &QPushButton::clicked, this, &PropertiesWidget::onSaveProject);
    connect(restoreBtn, &QPushButton::clicked, this, &PropertiesWidget::onRestoreProject);
}

// ========== POINT TAB ==========
void PropertiesWidget::setupPointTab()
{
    auto* tab = new QWidget(); auto* layout = new QVBoxLayout(tab);
    auto* nameRow = new QHBoxLayout();
    auto* nameLabel = new QLabel(QStringLiteral("Point Name"), tab);
    QFont bf = nameLabel->font(); bf.setBold(true); nameLabel->setFont(bf);
    nameRow->addWidget(nameLabel);
    m_ptWFStatusLabel = new QLabel(QStringLiteral("(Non WF)"), tab);
    QFont sf = m_ptWFStatusLabel->font(); sf.setBold(true); m_ptWFStatusLabel->setFont(sf);
    m_ptWFStatusLabel->setStyleSheet(QStringLiteral("color: navy;"));
    nameRow->addStretch(); nameRow->addWidget(m_ptWFStatusLabel); layout->addLayout(nameRow);
    m_ptNameEdit = new QLineEdit(tab); layout->addWidget(m_ptNameEdit);
    auto* riRow = new QHBoxLayout();
    riRow->addWidget(new QLabel(QStringLiteral("&Refractive Index:"), tab));
    m_ptRefIndexEdit = new QLineEdit(QStringLiteral("0"), tab); riRow->addWidget(m_ptRefIndexEdit);
    layout->addLayout(riRow);
    auto* ptGroup = new QGroupBox(QStringLiteral(" Point Coordinates "), tab);
    auto* ptLayout = new QFormLayout(ptGroup);
    m_ptXEdit = new QLineEdit(QStringLiteral("0"), ptGroup); ptLayout->addRow(QStringLiteral(" &X:"), m_ptXEdit);
    m_ptYEdit = new QLineEdit(QStringLiteral("0"), ptGroup); ptLayout->addRow(QStringLiteral(" &Y:"), m_ptYEdit);
    layout->addWidget(ptGroup);
    m_ptFlipNCheck = new QCheckBox(QStringLiteral("Flip normals"), tab); layout->addWidget(m_ptFlipNCheck);
    auto* btnRow = new QHBoxLayout();
    auto* saveBtn = new QPushButton(QStringLiteral("Save"), tab); saveBtn->setDefault(true);
    auto* restoreBtn = new QPushButton(QStringLiteral("Restore"), tab);
    btnRow->addStretch(); btnRow->addWidget(restoreBtn); btnRow->addWidget(saveBtn);
    layout->addLayout(btnRow); layout->addStretch();
    m_tabs->addTab(tab, QStringLiteral("Point"));
    connect(saveBtn, &QPushButton::clicked, this, &PropertiesWidget::onSavePoint);
    connect(restoreBtn, &QPushButton::clicked, this, &PropertiesWidget::onRestorePoint);
}

// ========== LINE TAB ==========
void PropertiesWidget::setupLineTab()
{
    auto* tab = new QWidget(); auto* layout = new QVBoxLayout(tab);
    auto* nameRow = new QHBoxLayout();
    auto* nl = new QLabel(QStringLiteral("Line Name"), tab);
    QFont bf = nl->font(); bf.setBold(true); nl->setFont(bf); nameRow->addWidget(nl);
    m_lnWFStatusLabel = new QLabel(QStringLiteral("(Non WF)"), tab);
    QFont sf = m_lnWFStatusLabel->font(); sf.setBold(true); m_lnWFStatusLabel->setFont(sf);
    m_lnWFStatusLabel->setStyleSheet(QStringLiteral("color: navy;"));
    nameRow->addStretch(); nameRow->addWidget(m_lnWFStatusLabel); layout->addLayout(nameRow);
    m_lnNameEdit = new QLineEdit(tab); layout->addWidget(m_lnNameEdit);
    auto* riRow = new QHBoxLayout();
    riRow->addWidget(new QLabel(QStringLiteral("&Refractive Index:"), tab));
    m_lnRefIndexEdit = new QLineEdit(QStringLiteral("0"), tab); riRow->addWidget(m_lnRefIndexEdit);
    layout->addLayout(riRow);
    auto* p1G = new QGroupBox(QStringLiteral(" First Point Coordinates "), tab);
    auto* p1L = new QFormLayout(p1G);
    m_lnP1XEdit = new QLineEdit(QStringLiteral("0"), p1G); p1L->addRow(QStringLiteral(" &X:"), m_lnP1XEdit);
    m_lnP1YEdit = new QLineEdit(QStringLiteral("0"), p1G); p1L->addRow(QStringLiteral(" &Y:"), m_lnP1YEdit);
    layout->addWidget(p1G);
    auto* p2G = new QGroupBox(QStringLiteral(" Second Point Coordinates "), tab);
    auto* p2L = new QFormLayout(p2G);
    m_lnP2XEdit = new QLineEdit(QStringLiteral("0"), p2G); p2L->addRow(QStringLiteral(" &X:"), m_lnP2XEdit);
    m_lnP2YEdit = new QLineEdit(QStringLiteral("0"), p2G); p2L->addRow(QStringLiteral(" &Y:"), m_lnP2YEdit);
    layout->addWidget(p2G);
    m_lnFlipNCheck = new QCheckBox(QStringLiteral("Flip normals"), tab); layout->addWidget(m_lnFlipNCheck);
    auto* btnRow = new QHBoxLayout();
    auto* saveBtn = new QPushButton(QStringLiteral("Save"), tab); saveBtn->setDefault(true);
    auto* restoreBtn = new QPushButton(QStringLiteral("Restore"), tab);
    btnRow->addStretch(); btnRow->addWidget(restoreBtn); btnRow->addWidget(saveBtn);
    layout->addLayout(btnRow); layout->addStretch();
    m_tabs->addTab(tab, QStringLiteral("Line"));
    connect(saveBtn, &QPushButton::clicked, this, &PropertiesWidget::onSaveLine);
    connect(restoreBtn, &QPushButton::clicked, this, &PropertiesWidget::onRestoreLine);
}

// ========== ARC TAB ==========
void PropertiesWidget::setupArcTab()
{
    auto* tab = new QWidget(); auto* layout = new QVBoxLayout(tab);
    auto* nameRow = new QHBoxLayout();
    auto* nl = new QLabel(QStringLiteral("Arc Name"), tab);
    QFont bf = nl->font(); bf.setBold(true); nl->setFont(bf); nameRow->addWidget(nl);
    m_arcWFStatusLabel = new QLabel(QStringLiteral("(Non WF)"), tab);
    QFont sf = m_arcWFStatusLabel->font(); sf.setBold(true); m_arcWFStatusLabel->setFont(sf);
    m_arcWFStatusLabel->setStyleSheet(QStringLiteral("color: navy;"));
    nameRow->addStretch(); nameRow->addWidget(m_arcWFStatusLabel); layout->addLayout(nameRow);
    m_arcNameEdit = new QLineEdit(tab); layout->addWidget(m_arcNameEdit);
    auto* riRow = new QHBoxLayout();
    riRow->addWidget(new QLabel(QStringLiteral("&Refractive Index:"), tab));
    m_arcRefIndexEdit = new QLineEdit(QStringLiteral("0"), tab); riRow->addWidget(m_arcRefIndexEdit);
    layout->addLayout(riRow);
    auto* cG = new QGroupBox(QStringLiteral(" Center Point Coordinates "), tab);
    auto* cL = new QFormLayout(cG);
    m_arcXEdit = new QLineEdit(QStringLiteral("0"), cG); cL->addRow(QStringLiteral(" &X:"), m_arcXEdit);
    m_arcYEdit = new QLineEdit(QStringLiteral("0"), cG); cL->addRow(QStringLiteral(" &Y:"), m_arcYEdit);
    layout->addWidget(cG);
    auto* aG = new QGroupBox(QStringLiteral(" Arc Parameters   "), tab);
    auto* aL = new QFormLayout(aG);
    m_arcRadiusEdit = new QLineEdit(QStringLiteral("0"), aG); aL->addRow(QStringLiteral(" Ra&dius:"), m_arcRadiusEdit);
    m_arcStartAngleEdit = new QLineEdit(QStringLiteral("0"), aG); aL->addRow(QStringLiteral(" &Start Angle:"), m_arcStartAngleEdit);
    m_arcEndAngleEdit = new QLineEdit(QStringLiteral("0"), aG); aL->addRow(QStringLiteral(" &End Angle:"), m_arcEndAngleEdit);
    layout->addWidget(aG);
    auto* amntRow = new QHBoxLayout();
    amntRow->addWidget(new QLabel(QStringLiteral(" &Number of control points:"), tab));
    m_arcAmntPtsEdit = new QLineEdit(QStringLiteral("0"), tab); amntRow->addWidget(m_arcAmntPtsEdit);
    layout->addLayout(amntRow);
    m_arcFlipNCheck = new QCheckBox(QStringLiteral("Flip normals"), tab); layout->addWidget(m_arcFlipNCheck);
    auto* btnRow = new QHBoxLayout();
    auto* saveBtn = new QPushButton(QStringLiteral("Save"), tab); saveBtn->setDefault(true);
    auto* restoreBtn = new QPushButton(QStringLiteral("Restore"), tab);
    btnRow->addStretch(); btnRow->addWidget(restoreBtn); btnRow->addWidget(saveBtn);
    layout->addLayout(btnRow); layout->addStretch();
    m_tabs->addTab(tab, QStringLiteral("Arc"));
    connect(saveBtn, &QPushButton::clicked, this, &PropertiesWidget::onSaveArc);
    connect(restoreBtn, &QPushButton::clicked, this, &PropertiesWidget::onRestoreArc);
}

// ========== CURVE TAB ==========
void PropertiesWidget::setupCurveTab()
{
    auto* tab = new QWidget(); auto* layout = new QVBoxLayout(tab);
    auto* nameRow = new QHBoxLayout();
    auto* nl = new QLabel(QStringLiteral("Curve Name"), tab);
    QFont bf = nl->font(); bf.setBold(true); nl->setFont(bf); nameRow->addWidget(nl);
    m_cvWFStatusLabel = new QLabel(QStringLiteral("(Non WF)"), tab);
    QFont sf = m_cvWFStatusLabel->font(); sf.setBold(true); m_cvWFStatusLabel->setFont(sf);
    m_cvWFStatusLabel->setStyleSheet(QStringLiteral("color: navy;"));
    nameRow->addStretch(); nameRow->addWidget(m_cvWFStatusLabel); layout->addLayout(nameRow);
    m_cvNameEdit = new QLineEdit(tab); layout->addWidget(m_cvNameEdit);
    auto* riRow = new QHBoxLayout();
    riRow->addWidget(new QLabel(QStringLiteral("&Refractive Index:"), tab));
    m_cvRefIndexEdit = new QLineEdit(QStringLiteral("0"), tab); riRow->addWidget(m_cvRefIndexEdit);
    layout->addLayout(riRow);
    auto* ptsGroup = new QGroupBox(QStringLiteral(" List of Points Coordinates "), tab);
    auto* ptsLayout = new QVBoxLayout(ptsGroup);
    auto* headerRow = new QHBoxLayout();
    headerRow->addStretch(3); headerRow->addWidget(new QLabel(QStringLiteral(" X"), ptsGroup));
    headerRow->addStretch(1); headerRow->addWidget(new QLabel(QStringLiteral(" Y"), ptsGroup)); headerRow->addStretch(3);
    ptsLayout->addLayout(headerRow);
    m_cvGrid = new QTableWidget(0, 2, ptsGroup);
    m_cvGrid->setHorizontalHeaderLabels({QStringLiteral("X"), QStringLiteral("Y")});
    m_cvGrid->horizontalHeader()->setStretchLastSection(true);
    m_cvGrid->setSelectionBehavior(QAbstractItemView::SelectRows);
    ptsLayout->addWidget(m_cvGrid); layout->addWidget(ptsGroup);
    auto* btnRow = new QHBoxLayout();
    m_cvAddBtn = new QPushButton(QStringLiteral("&Add"), tab); m_cvAddBtn->setEnabled(false); m_cvAddBtn->setVisible(false);
    m_cvDelBtn = new QPushButton(QStringLiteral("&Delete"), tab); m_cvDelBtn->setEnabled(false); m_cvDelBtn->setVisible(false);
    btnRow->addWidget(m_cvAddBtn); btnRow->addWidget(m_cvDelBtn); btnRow->addStretch();
    layout->addLayout(btnRow);
    m_cvEditCurveBtn = new QPushButton(QStringLiteral("Edit cur&ve"), tab); layout->addWidget(m_cvEditCurveBtn);
    m_cvFlipNCheck = new QCheckBox(QStringLiteral("Flip normals"), tab); layout->addWidget(m_cvFlipNCheck);
    auto* btnRow2 = new QHBoxLayout();
    auto* saveBtn = new QPushButton(QStringLiteral("Save"), tab); saveBtn->setDefault(true);
    auto* restoreBtn = new QPushButton(QStringLiteral("Restore"), tab);
    btnRow2->addStretch(); btnRow2->addWidget(restoreBtn); btnRow2->addWidget(saveBtn);
    layout->addLayout(btnRow2); layout->addStretch();
    m_tabs->addTab(tab, QStringLiteral("Curve"));
    connect(saveBtn, &QPushButton::clicked, this, &PropertiesWidget::onSaveCurve);
    connect(restoreBtn, &QPushButton::clicked, this, &PropertiesWidget::onRestoreCurve);
}

// ========== OBJECTS TAB (matching tshObjects in Ovals Designer) ==========
void PropertiesWidget::setupObjectsTab()
{
    auto* tab = new QWidget(); auto* layout = new QVBoxLayout(tab);
    auto* nameLabel = new QLabel(QStringLiteral("Objects Name"), tab);
    QFont bf = nameLabel->font(); bf.setBold(true); nameLabel->setFont(bf);
    layout->addWidget(nameLabel);
    m_objListWidget = new QListWidget(tab); layout->addWidget(m_objListWidget);
    auto* btnRow = new QHBoxLayout();
    auto* insertBtn = new QPushButton(QStringLiteral("&Insert"), tab);
    auto* deleteBtn = new QPushButton(QStringLiteral("&Delete"), tab);
    auto* selectBtn = new QPushButton(QStringLiteral("&Select"), tab); selectBtn->setDefault(true);
    btnRow->addWidget(insertBtn); btnRow->addWidget(deleteBtn); btnRow->addStretch(); btnRow->addWidget(selectBtn);
    layout->addLayout(btnRow);
    m_tabs->addTab(tab, QStringLiteral("Objects"));
    connect(insertBtn, &QPushButton::clicked, this, &PropertiesWidget::insertObjectRequested);
    connect(deleteBtn, &QPushButton::clicked, this, &PropertiesWidget::deleteObjectRequested);
    connect(selectBtn, &QPushButton::clicked, this, [this]() {
        if (m_objListWidget->currentItem())
            emit selectObjectRequested(m_objListWidget->currentItem()->text());
    });
}

// ========== CALC OVAL TAB (matching tshCalcOval in Ovals Designer) ==========
void PropertiesWidget::setupCalcOvalTab()
{
    auto* tab = new QWidget(); auto* layout = new QVBoxLayout(tab);
    auto* titleLabel = new QLabel(QStringLiteral("Cartesian Oval parameters"), tab);
    QFont bf = titleLabel->font(); bf.setBold(true); titleLabel->setFont(bf);
    layout->addWidget(titleLabel);
    m_cOvalNameEdit = new QLineEdit(tab); m_cOvalNameEdit->setReadOnly(true);
    layout->addWidget(m_cOvalNameEdit);
    auto* form = new QFormLayout();
    m_cOvalWF1Edit = new QLineEdit(tab); m_cOvalWF1Edit->setReadOnly(true);
    form->addRow(QStringLiteral("First WF:"), m_cOvalWF1Edit);
    m_cOvalWF2Edit = new QLineEdit(tab); m_cOvalWF2Edit->setReadOnly(true);
    form->addRow(QStringLiteral("Second WF:"), m_cOvalWF2Edit);
    m_cOvalRefEdit = new QLineEdit(tab); m_cOvalRefEdit->setReadOnly(true);
    form->addRow(QStringLiteral("Ref. Point:"), m_cOvalRefEdit);
    m_cOvalQtyPtsEdit = new QLineEdit(tab);
    form->addRow(QStringLiteral("Calculations Qty:"), m_cOvalQtyPtsEdit);
    layout->addLayout(form);
    auto* sep = new QFrame(tab); sep->setFrameShape(QFrame::HLine); sep->setFrameShadow(QFrame::Sunken);
    layout->addWidget(sep);
    layout->addWidget(new QLabel(QStringLiteral("Result object name:"), tab));
    m_cOvalResultEdit = new QLineEdit(tab); m_cOvalResultEdit->setReadOnly(true);
    layout->addWidget(m_cOvalResultEdit);
    auto* btnRow = new QHBoxLayout();
    auto* saveBtn = new QPushButton(QStringLiteral("Save"), tab); saveBtn->setDefault(true);
    auto* restoreBtn = new QPushButton(QStringLiteral("Restore"), tab);
    btnRow->addStretch(); btnRow->addWidget(restoreBtn); btnRow->addWidget(saveBtn);
    layout->addLayout(btnRow); layout->addStretch();
    m_tabs->addTab(tab, QStringLiteral("CalcOval"));
    connect(saveBtn, &QPushButton::clicked, this, &PropertiesWidget::calculateOvalRequested);
    connect(restoreBtn, &QPushButton::clicked, this, &PropertiesWidget::onRestoreCalcOval);
}

// ========== PROPAGATE TAB (matching tshPropagate in Ovals Designer) ==========
void PropertiesWidget::setupPropagateTab()
{
    auto* tab = new QWidget(); auto* layout = new QVBoxLayout(tab);
    auto* titleLabel = new QLabel(QStringLiteral("WF Propagation parammeters"), tab);
    QFont bf = titleLabel->font(); bf.setBold(true); titleLabel->setFont(bf);
    layout->addWidget(titleLabel);
    m_propgNameEdit = new QLineEdit(tab); m_propgNameEdit->setReadOnly(true);
    layout->addWidget(m_propgNameEdit);
    auto* form = new QFormLayout();
    m_propgWFEdit = new QLineEdit(tab); m_propgWFEdit->setReadOnly(true);
    form->addRow(QStringLiteral("Wave-Front"), m_propgWFEdit);
    m_propgIOREdit = new QLineEdit(tab); m_propgIOREdit->setReadOnly(true);
    form->addRow(QStringLiteral("Refract. Indx:"), m_propgIOREdit);
    m_propgSurfEdit = new QLineEdit(tab); m_propgSurfEdit->setReadOnly(true);
    form->addRow(QStringLiteral("Surface:"), m_propgSurfEdit);
    m_propgQtyPtsEdit = new QLineEdit(tab);
    form->addRow(QStringLiteral("Calculations Qty:"), m_propgQtyPtsEdit);
    m_propgOffsetEdit = new QLineEdit(tab);
    form->addRow(QStringLiteral("OPL Offset:"), m_propgOffsetEdit);
    layout->addLayout(form);
    auto* sep = new QFrame(tab); sep->setFrameShape(QFrame::HLine); sep->setFrameShadow(QFrame::Sunken);
    layout->addWidget(sep);
    layout->addWidget(new QLabel(QStringLiteral("Result object name:"), tab));
    m_propgResultEdit = new QLineEdit(tab); m_propgResultEdit->setReadOnly(true);
    layout->addWidget(m_propgResultEdit);
    auto* btnRow = new QHBoxLayout();
    auto* saveBtn = new QPushButton(QStringLiteral("Save"), tab); saveBtn->setDefault(true);
    auto* restoreBtn = new QPushButton(QStringLiteral("Restore"), tab);
    btnRow->addStretch(); btnRow->addWidget(restoreBtn); btnRow->addWidget(saveBtn);
    layout->addLayout(btnRow); layout->addStretch();
    m_tabs->addTab(tab, QStringLiteral("Propagate"));
    connect(saveBtn, &QPushButton::clicked, this, &PropertiesWidget::propagateWFRequested);
    connect(restoreBtn, &QPushButton::clicked, this, &PropertiesWidget::onRestorePropagate);
}

// ========== setObject / showObjectTabs ==========
void PropertiesWidget::setObject(CustomObject* obj)
{
    m_currentObject = obj;
    showObjectTabs(obj);
}

void PropertiesWidget::setProject(Project* project)
{
    m_currentProject = project;
    if (project) {
        m_prjNameEdit->setText(project->name());
        m_prjXAutoCheck->setEnabled(true);
        m_prjYAutoCheck->setEnabled(true);
        m_objListWidget->clear();
        for (auto* o : project->dataObjects()) if (o) m_objListWidget->addItem(o->name());
        for (auto* o : project->resultObjects()) if (o) m_objListWidget->addItem(o->name());
    }
    m_tabs->setCurrentIndex(0);
}

void PropertiesWidget::setOperations(Project* project)
{
    Q_UNUSED(project);
}

void PropertiesWidget::refresh()
{
    if (m_currentProject) setProject(m_currentProject);
    if (m_currentObject) showObjectTabs(m_currentObject);
}

void PropertiesWidget::showObjectTabs(CustomObject* obj)
{
    if (!obj) { m_tabs->setCurrentIndex(0); return; }

    auto updateWFLabel = [](QLabel* label, ObjectType type) {
        if (isWavefront(type)) {
            label->setText(isVirtualWF(type) ? QStringLiteral("(WF Virtual)") : QStringLiteral("(WF)"));
            label->setStyleSheet(QStringLiteral("color: maroon; font-weight: bold;"));
        } else {
            label->setText(QStringLiteral("(Non WF)"));
            label->setStyleSheet(QStringLiteral("color: navy; font-weight: bold;"));
        }
    };

    auto baseType = toBaseType(obj->objectType());
    switch (baseType) {
    case 0x001: { // Point
        m_ptNameEdit->setText(obj->name());
        m_ptRefIndexEdit->setText(QString::number(obj->refractiveIndex()));
        updateWFLabel(m_ptWFStatusLabel, obj->objectType());
        if (!obj->controlPoints().isEmpty()) {
            m_ptXEdit->setText(QString::number(obj->controlPoints().first().x(), 'f', 6));
            m_ptYEdit->setText(QString::number(obj->controlPoints().first().y(), 'f', 6));
        }
        m_tabs->setCurrentIndex(1); break;
    }
    case 0x002: { // Line
        m_lnNameEdit->setText(obj->name());
        m_lnRefIndexEdit->setText(QString::number(obj->refractiveIndex()));
        updateWFLabel(m_lnWFStatusLabel, obj->objectType());
        const auto& lnPts = obj->controlPoints();
        if (lnPts.size() >= 2) {
            m_lnP1XEdit->setText(QString::number(lnPts[0].x(), 'f', 6));
            m_lnP1YEdit->setText(QString::number(lnPts[0].y(), 'f', 6));
            m_lnP2XEdit->setText(QString::number(lnPts[1].x(), 'f', 6));
            m_lnP2YEdit->setText(QString::number(lnPts[1].y(), 'f', 6));
        }
        m_tabs->setCurrentIndex(2); break;
    }
    case 0x003: { // Arc
        m_arcNameEdit->setText(obj->name());
        m_arcRefIndexEdit->setText(QString::number(obj->refractiveIndex()));
        updateWFLabel(m_arcWFStatusLabel, obj->objectType());
        const auto& arcPts = obj->controlPoints();
        if (!arcPts.isEmpty()) {
            m_arcXEdit->setText(QString::number(arcPts.first().x(), 'f', 6));
            m_arcYEdit->setText(QString::number(arcPts.first().y(), 'f', 6));
        }
        auto* arcObj = dynamic_cast<ArcObject*>(obj);
        if (arcObj) {
            m_arcRadiusEdit->setText(QString::number(arcObj->radius()));
            m_arcStartAngleEdit->setText(QString::number(arcObj->startAngle()));
            m_arcEndAngleEdit->setText(QString::number(arcObj->endAngle()));
            m_arcAmntPtsEdit->setText(QStringLiteral("50"));
        }
        m_tabs->setCurrentIndex(3); break;
    }
    default: { // Curve
        m_cvNameEdit->setText(obj->name());
        m_cvRefIndexEdit->setText(QString::number(obj->refractiveIndex()));
        updateWFLabel(m_cvWFStatusLabel, obj->objectType());
        m_cvGrid->setRowCount(0);
        for (const auto& pt : obj->controlPoints()) {
            int row = m_cvGrid->rowCount();
            m_cvGrid->insertRow(row);
            m_cvGrid->setItem(row, 0, new QTableWidgetItem(QString::number(pt.x(), 'f', 6)));
            m_cvGrid->setItem(row, 1, new QTableWidgetItem(QString::number(pt.y(), 'f', 6)));
        }
        m_tabs->setCurrentIndex(4); break;
    }
    }
}

// ========== SAVE/RESTORE SLOTS ==========
void PropertiesWidget::onSaveProject() {
    if (m_currentProject) { m_currentProject->setName(m_prjNameEdit->text()); emit projectModified(m_currentProject); }
}
void PropertiesWidget::onRestoreProject() { if (m_currentProject) m_prjNameEdit->setText(m_currentProject->name()); }
void PropertiesWidget::onSavePoint() { if (m_currentObject) emit objectModified(m_currentObject); }
void PropertiesWidget::onRestorePoint() { if (m_currentObject) showObjectTabs(m_currentObject); }
void PropertiesWidget::onSaveLine() { if (m_currentObject) emit objectModified(m_currentObject); }
void PropertiesWidget::onRestoreLine() { if (m_currentObject) showObjectTabs(m_currentObject); }
void PropertiesWidget::onSaveArc() { if (m_currentObject) emit objectModified(m_currentObject); }
void PropertiesWidget::onRestoreArc() { if (m_currentObject) showObjectTabs(m_currentObject); }
void PropertiesWidget::onSaveCurve() { if (m_currentObject) emit objectModified(m_currentObject); }
void PropertiesWidget::onRestoreCurve() { if (m_currentObject) showObjectTabs(m_currentObject); }
void PropertiesWidget::onSaveCalcOval() { emit calculateOvalRequested(); }
void PropertiesWidget::onRestoreCalcOval() {}
void PropertiesWidget::onSavePropagate() { emit propagateWFRequested(); }
void PropertiesWidget::onRestorePropagate() {}

} // namespace ExpressDesigner