#include "InsertObjectDialog.h"
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <core/ObjectFactory.h>

namespace ExpressDesigner {

InsertObjectDialog::InsertObjectDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Insert Object"));
    auto* layout = new QVBoxLayout(this);
    auto* form = new QFormLayout();
    form->addRow(QStringLiteral("Type:"), new QLabel(QStringLiteral("Select object type")));
    layout->addLayout(form);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
        m_createdObject = ObjectFactory::createFromType(ObjectType::Point, QStringLiteral("NewObject"), false, this);
        accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

} // namespace ExpressDesigner