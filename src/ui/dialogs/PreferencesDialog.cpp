#include "PreferencesDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QSettings>

namespace ExpressDesigner {

PreferencesDialog::PreferencesDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Preferences"));

    auto* layout = new QVBoxLayout(this);

    auto* form = new QFormLayout();

    auto* autoZoomChk = new QCheckBox(this);
    QSettings settings;
    autoZoomChk->setChecked(settings.value(QStringLiteral("Preferences/autoZoom"), true).toBool());
    form->addRow(QStringLiteral("Auto-zoom on insert:"), autoZoomChk);

    auto* normalsQtySpin = new QSpinBox(this);
    normalsQtySpin->setRange(1, 1000);
    normalsQtySpin->setValue(settings.value(QStringLiteral("Preferences/defaultNormalsQty"), 10).toInt());
    form->addRow(QStringLiteral("Default normals quantity:"), normalsQtySpin);

    auto* normalsLenSpin = new QDoubleSpinBox(this);
    normalsLenSpin->setRange(0.1, 100.0);
    normalsLenSpin->setDecimals(2);
    normalsLenSpin->setValue(settings.value(QStringLiteral("Preferences/defaultNormalsLen"), 1.0).toDouble());
    form->addRow(QStringLiteral("Default normals length:"), normalsLenSpin);

    auto* openLastChk = new QCheckBox(this);
    openLastChk->setChecked(settings.value(QStringLiteral("Preferences/openLastProjectAutomatically"), false).toBool());
    form->addRow(QStringLiteral("Open last project automatically:"), openLastChk);

    layout->addLayout(form);
    layout->addStretch();

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, [this, autoZoomChk, normalsQtySpin, normalsLenSpin, openLastChk]() {
        QSettings settings;
        settings.setValue(QStringLiteral("Preferences/autoZoom"), autoZoomChk->isChecked());
        settings.setValue(QStringLiteral("Preferences/defaultNormalsQty"), normalsQtySpin->value());
        settings.setValue(QStringLiteral("Preferences/defaultNormalsLen"), normalsLenSpin->value());
        settings.setValue(QStringLiteral("Preferences/openLastProjectAutomatically"), openLastChk->isChecked());
        accept();
    });

    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

} // namespace ExpressDesigner