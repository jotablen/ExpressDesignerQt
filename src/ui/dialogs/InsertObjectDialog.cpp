#include "InsertObjectDialog.h"
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <core/ObjectFactory.h>

namespace ExpressDesigner {

InsertObjectDialog::InsertObjectDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Insert Object Dialog"));

    auto* layout = new QVBoxLayout(this);

    // --- Type combo ---
    auto* form = new QFormLayout();
    m_typeCombo = new QComboBox(this);
    m_typeCombo->addItem(QStringLiteral("Point Object"));
    m_typeCombo->addItem(QStringLiteral("Line Object"));
    m_typeCombo->addItem(QStringLiteral("Arc Object"));
    m_typeCombo->addItem(QStringLiteral("Curve Object"));
    m_typeCombo->setCurrentIndex(0);
    form->addRow(QStringLiteral("Type:"), m_typeCombo);

    // --- Name edit ---
    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setText(QStringLiteral("NewObject"));
    form->addRow(QStringLiteral("Name:"), m_nameEdit);

    layout->addLayout(form);

    // --- Is WF checkbox ---
    m_isWfCheck = new QCheckBox(QStringLiteral("Object is a WF"), this);
    m_isWfCheck->setChecked(true);
    layout->addWidget(m_isWfCheck);

    // --- Is Virtual WF checkbox ---
    m_isVirtualWfCheck = new QCheckBox(QStringLiteral("Wavefront is virtual"), this);
    m_isVirtualWfCheck->setChecked(false);
    m_isVirtualWfCheck->setVisible(false); // hidden by default (matches original)
    layout->addWidget(m_isVirtualWfCheck);

    connect(m_isWfCheck, &QCheckBox::toggled, this, &InsertObjectDialog::onIsWFChanged);

    // --- Buttons ---
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
        createObject();
        if (m_createdObject)
            accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

void InsertObjectDialog::onIsWFChanged(bool checked)
{
    // Enable the virtual WF checkbox only when "Is WF" is checked.
    // When disabled, also uncheck it (matches original behavior).
    m_isVirtualWfCheck->setEnabled(checked);
    if (!checked)
        m_isVirtualWfCheck->setChecked(false);
}

void InsertObjectDialog::createObject()
{
    // Map combo index to ObjectType base
    ObjectType baseType;
    switch (m_typeCombo->currentIndex()) {
    case 0: baseType = ObjectType::Point; break;
    case 1: baseType = ObjectType::Line;  break;
    case 2: baseType = ObjectType::Arc;   break;
    case 3:
    default: baseType = ObjectType::Curve; break;
    }

    // Apply WF / VirtualWF masks
    ObjectType finalType = baseType;
    if (m_isWfCheck->isChecked()) {
        if (m_isVirtualWfCheck->isChecked())
            finalType = withVirtualWF(baseType);
        else
            finalType = withWavefront(baseType);
    }

    QString name = m_nameEdit->text().trimmed();
    if (name.isEmpty())
        name = QStringLiteral("NewObject");

    bool isWF = m_isWfCheck->isChecked();
    m_createdObject = ObjectFactory::createFromType(finalType, name, isWF, this);
}

} // namespace ExpressDesigner