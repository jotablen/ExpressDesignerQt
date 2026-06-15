#include "TranslateObjectDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QFrame>
#include <QMessageBox>

namespace ExpressDesigner {

TranslateObjectDialog::TranslateObjectDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Translate Object"));
    setMinimumWidth(400);

    auto* layout = new QVBoxLayout(this);

    auto* selLayout = new QHBoxLayout();
    selLayout->addWidget(new QLabel(QStringLiteral("Object:"), this));
    m_objectCombo = new QComboBox(this);
    m_objectCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    selLayout->addWidget(m_objectCombo);
    layout->addLayout(selLayout);

    auto* sep1 = new QFrame(this);
    sep1->setFrameShape(QFrame::HLine);
    sep1->setFrameShadow(QFrame::Sunken);
    layout->addWidget(sep1);

    auto* deltaLayout = new QHBoxLayout();
    deltaLayout->addWidget(new QLabel(QStringLiteral("ΔX:"), this));
    m_deltaXEdit = new QLineEdit(QStringLiteral("0"), this);
    m_deltaXEdit->setFixedWidth(100);
    deltaLayout->addWidget(m_deltaXEdit);
    deltaLayout->addWidget(new QLabel(QStringLiteral("ΔY:"), this));
    m_deltaYEdit = new QLineEdit(QStringLiteral("0"), this);
    m_deltaYEdit->setFixedWidth(100);
    deltaLayout->addWidget(m_deltaYEdit);
    layout->addLayout(deltaLayout);

    layout->addStretch();

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
        if (m_objectCombo->currentIndex() < 0) {
            QMessageBox::critical(this, QStringLiteral("ERROR!!!"),
                QStringLiteral("Please select an object to translate."));
            return;
        }
        bool okX = false, okY = false;
        m_deltaXEdit->text().toDouble(&okX);
        m_deltaYEdit->text().toDouble(&okY);
        if (!okX || !okY) {
            QMessageBox::critical(this, QStringLiteral("ERROR!!!"),
                QStringLiteral("ΔX and ΔY must be numbers."));
            return;
        }
        accept();
    });

    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void TranslateObjectDialog::setProject(Project* project)
{
    m_project = project;
    populateCombo(project);

    if (m_selectedObj) {
        int idx = m_objectCombo->findText(m_selectedObj->name());
        if (idx >= 0)
            m_objectCombo->setCurrentIndex(idx);
    } else if (m_objectCombo->count() > 0) {
        m_objectCombo->setCurrentIndex(0);
    }
}

void TranslateObjectDialog::setSelectedObject(CustomObject* obj)
{
    m_selectedObj = obj;
}

void TranslateObjectDialog::populateCombo(Project* project)
{
    m_objectCombo->clear();
    if (!project) return;

    for (auto* obj : project->dataObjects()) {
        if (!obj) continue;
        m_objectCombo->addItem(obj->name(), QVariant::fromValue(reinterpret_cast<quintptr>(obj)));
    }
    for (auto* obj : project->resultObjects()) {
        if (!obj) continue;
        m_objectCombo->addItem(obj->name(), QVariant::fromValue(reinterpret_cast<quintptr>(obj)));
    }
}

} // namespace ExpressDesigner