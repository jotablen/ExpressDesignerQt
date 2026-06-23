#include "CalcOvalDialog.h"
#include <core/RefPointDescriptor.h>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QFrame>
#include <QMessageBox>

namespace ExpressDesigner {

CalcOvalDialog::CalcOvalDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Carthesian oval calculation"));

    auto* layout = new QVBoxLayout(this);

    // --- Wavefronts section ---
    auto* form = new QFormLayout();

    m_wfOrgCombo = new QComboBox(this);
    m_wfOrgCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    form->addRow(QStringLiteral("First wavefront:"), m_wfOrgCombo);

    m_wfDestCombo = new QComboBox(this);
    m_wfDestCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    form->addRow(QStringLiteral("Second wavefront:"), m_wfDestCombo);

    layout->addLayout(form);

    auto* sep1 = new QFrame(this);
    sep1->setFrameShape(QFrame::HLine);
    sep1->setFrameShadow(QFrame::Sunken);
    layout->addWidget(sep1);

    // --- Reference point & amount ---
    auto* form2 = new QFormLayout();

    m_refPointCombo = new QComboBox(this);
    form2->addRow(QStringLiteral("Reference Point:"), m_refPointCombo);

    m_amountEdit = new QLineEdit(QStringLiteral("100"), this);
    form2->addRow(QStringLiteral("Control points per wavefront:"), m_amountEdit);

    layout->addLayout(form2);

    auto* sep2 = new QFrame(this);
    sep2->setFrameShape(QFrame::HLine);
    sep2->setFrameShadow(QFrame::Sunken);
    layout->addWidget(sep2);

    // --- Result name ---
    m_resultNameEdit = new QLineEdit(this);
    layout->addWidget(new QLabel(QStringLiteral("Result object name:"), this));
    layout->addWidget(m_resultNameEdit);

    auto* sep3 = new QFrame(this);
    sep3->setFrameShape(QFrame::HLine);
    sep3->setFrameShadow(QFrame::Sunken);
    layout->addWidget(sep3);

    // --- Exclude RTI checkbox (hidden by default, matching original) ---
    m_excludeRtiCheck = new QCheckBox(QStringLiteral("Exclude TI&R points from result"), this);
    m_excludeRtiCheck->setChecked(true);
    m_excludeRtiCheck->setVisible(false);
    layout->addWidget(m_excludeRtiCheck);

    // --- Buttons ---
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
        // Validation matching original CalcOvalSimple::btnOkClick
        if (m_wfOrgCombo->currentIndex() < 0 || m_wfDestCombo->currentIndex() < 0) {
            QMessageBox::critical(this, QStringLiteral("ERROR!!! Selecting WFs"),
                QStringLiteral("Please, to realize the calculation, you must \nselect both WF Objects (Origin and Destiny)."));
            return;
        }
        if (m_wfOrgCombo->currentIndex() == m_wfDestCombo->currentIndex()) {
            QMessageBox::critical(this, QStringLiteral("ERROR!!! Selecting WFs"),
                QStringLiteral("Please, to realize the calculation, you must \nselect a different WF (Origin - Destiny)."));
            return;
        }
        if (m_resultNameEdit->text().trimmed().isEmpty()) {
            QMessageBox::critical(this, QStringLiteral("ERROR!!! Missing Object Name"),
                QStringLiteral("Please, enter a name to the \nResult Object."));
            m_resultNameEdit->setFocus();
            return;
        }
        bool ok = false;
        int amount = m_amountEdit->text().toInt(&ok);
        if (!ok || amount <= 0) {
            QMessageBox::critical(this, QStringLiteral("ERROR!!! Setting the amount of points"),
                QStringLiteral("Please, to calculate, \nenter only positives integer number."));
            m_amountEdit->setFocus();
            return;
        }
        m_excludeRtiCheck->setChecked(true);
        accept();
    });

    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    connect(m_wfOrgCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CalcOvalDialog::onWfOrgChanged);
    connect(m_wfDestCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CalcOvalDialog::onWfDestChanged);
}

void CalcOvalDialog::setProject(Project* project)
{
    m_project = project;
    populateCombos(project);

    // Default selections matching FormShow: idx 0 for origin, idx 1 for dest (or next available)
    if (m_wfOrgCombo->count() > 0)
        m_wfOrgCombo->setCurrentIndex(0);
    if (m_wfDestCombo->count() > 1) {
        // Select second item if origin=0, else select 0
        m_wfDestCombo->setCurrentIndex(1);
        // If origin==dest after selection, rotate dest
        if (m_wfDestCombo->currentIndex() == m_wfOrgCombo->currentIndex()) {
            if (m_wfDestCombo->currentIndex() < m_wfDestCombo->count() - 1)
                m_wfDestCombo->setCurrentIndex(m_wfDestCombo->currentIndex() + 1);
            else
                m_wfDestCombo->setCurrentIndex(0);
        }
    } else if (m_wfDestCombo->count() > 0) {
        m_wfDestCombo->setCurrentIndex(0);
    }
    if (m_refPointCombo->count() > 0)
        m_refPointCombo->setCurrentIndex(0);

    m_excludeRtiCheck->setChecked(true);
    updateResultName();
}

void CalcOvalDialog::populateCombos(Project* project)
{
    m_wfOrgCombo->clear();
    m_wfDestCombo->clear();
    m_refPointCombo->clear();

    if (!project) return;

    // Populate WF combo (all wavefront objects: data + result)
    const auto& dataObjects = project->dataObjects();
    for (CustomObject* obj : dataObjects) {
        if (!obj) continue;
        if (isWavefront(obj->objectType())) {
            m_wfOrgCombo->addItem(obj->name(), QVariant::fromValue(reinterpret_cast<quintptr>(obj)));
            m_wfDestCombo->addItem(obj->name(), QVariant::fromValue(reinterpret_cast<quintptr>(obj)));
        }
    }
    const auto& resultObjects = project->resultObjects();
    for (CustomObject* obj : resultObjects) {
        if (!obj) continue;
        if (isWavefront(obj->objectType())) {
            m_wfOrgCombo->addItem(obj->name(), QVariant::fromValue(reinterpret_cast<quintptr>(obj)));
            m_wfDestCombo->addItem(obj->name(), QVariant::fromValue(reinterpret_cast<quintptr>(obj)));
        }
    }

    // Populate reference points (only Point objects that are NOT wavefronts)
    auto addPoint = [this](CustomObject* obj) {
        if (toBaseType(obj->objectType()) == 0x001 && !isWavefront(obj->objectType())) {
            const auto& pts = obj->controlPoints();
            QPointF pt = pts.isEmpty() ? QPointF() : pts.first();
            QString label = obj->name() + QStringLiteral(" (%1, %2)")
                .arg(pt.x(), 0, 'f', 3).arg(pt.y(), 0, 'f', 3);
            QVariantMap map;
            map[QStringLiteral("kind")] = RefPointDescriptor::PointObject;
            map[QStringLiteral("name")] = obj->name();
            m_refPointCombo->addItem(label, map);
        }
    };
    for (auto* obj : dataObjects) addPoint(obj);
    for (auto* obj : resultObjects) addPoint(obj);

    // Populate Begin/End of non-WF Curves, Lines, and Arcs
    auto addCurveEndpoints = [this](CustomObject* obj) {
        if (!obj) return;
        auto base = toBaseType(obj->objectType());
        if ((base == static_cast<uint16_t>(ObjectType::Curve) ||
             base == static_cast<uint16_t>(ObjectType::Line) ||
             base == static_cast<uint16_t>(ObjectType::Arc)) &&
            !isWavefront(obj->objectType()) &&
            obj->controlPointCount() >= 2)
        {
            const auto& pts = obj->controlPoints();
            QPointF ptBegin = pts.first();
            QPointF ptEnd = pts.last();

            QString labelBegin = obj->name() + QStringLiteral(" (Begin) (%1, %2)")
                .arg(ptBegin.x(), 0, 'f', 3).arg(ptBegin.y(), 0, 'f', 3);
            QVariantMap mapBegin;
            mapBegin[QStringLiteral("kind")] = RefPointDescriptor::CurveBegin;
            mapBegin[QStringLiteral("name")] = obj->name();
            m_refPointCombo->addItem(labelBegin, mapBegin);

            QString labelEnd = obj->name() + QStringLiteral(" (End) (%1, %2)")
                .arg(ptEnd.x(), 0, 'f', 3).arg(ptEnd.y(), 0, 'f', 3);
            QVariantMap mapEnd;
            mapEnd[QStringLiteral("kind")] = RefPointDescriptor::CurveEnd;
            mapEnd[QStringLiteral("name")] = obj->name();
            m_refPointCombo->addItem(labelEnd, mapEnd);
        }
    };
    for (auto* obj : dataObjects) addCurveEndpoints(obj);
    for (auto* obj : resultObjects) addCurveEndpoints(obj);
}

int CalcOvalDialog::refPointKind() const
{
    if (m_refPointCombo->currentIndex() < 0) return 0;
    QVariant data = m_refPointCombo->currentData();
    if (!data.isValid()) return 0;
    // Store kind + sourceName as userData (int, QString)
    return data.toMap().value(QStringLiteral("kind")).toInt();
}

QString CalcOvalDialog::refPointSourceName() const
{
    if (m_refPointCombo->currentIndex() < 0) return QString();
    QVariant data = m_refPointCombo->currentData();
    if (!data.isValid()) return QString();
    return data.toMap().value(QStringLiteral("name")).toString();
}

void CalcOvalDialog::onWfOrgChanged()
{
    // If origin == dest, rotate dest (matching original)
    if (m_wfOrgCombo->currentText() == m_wfDestCombo->currentText()) {
        if (m_wfDestCombo->currentIndex() < m_wfDestCombo->count() - 1)
            m_wfDestCombo->setCurrentIndex(m_wfDestCombo->currentIndex() + 1);
        else
            m_wfDestCombo->setCurrentIndex(0);
    }
    updateResultName();
}

void CalcOvalDialog::onWfDestChanged()
{
    updateResultName();
}

void CalcOvalDialog::updateResultName()
{
    if (m_wfOrgCombo->currentIndex() >= 0 && m_wfDestCombo->currentIndex() >= 0) {
        QString name = QStringLiteral("Oval from: %1 - %2")
            .arg(m_wfOrgCombo->currentText(), m_wfDestCombo->currentText());
        m_resultNameEdit->setText(name);
    }
}

} // namespace ExpressDesigner