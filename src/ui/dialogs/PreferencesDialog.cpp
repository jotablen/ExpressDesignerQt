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

    auto* filterOutlierChk = new QCheckBox(this);
    filterOutlierChk->setChecked(settings.value(QStringLiteral("Preferences/filterOutlierPoints"), false).toBool());
    form->addRow(QStringLiteral("Filter outlier points (normal angle):"), filterOutlierChk);

    auto* outlierAlphaSpin = new QDoubleSpinBox(this);
    outlierAlphaSpin->setRange(0.1, 180.0);
    outlierAlphaSpin->setDecimals(1);
    outlierAlphaSpin->setValue(settings.value(QStringLiteral("Preferences/outlierAlphaDegrees"), 30.0).toDouble());
    outlierAlphaSpin->setSuffix(QStringLiteral("°"));
    outlierAlphaSpin->setEnabled(filterOutlierChk->isChecked());
    form->addRow(QStringLiteral("Outlier Alpha (degrees):"), outlierAlphaSpin);

    QObject::connect(filterOutlierChk, &QCheckBox::toggled, outlierAlphaSpin, &QDoubleSpinBox::setEnabled);

    layout->addLayout(form);
    layout->addStretch();

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, [this, autoZoomChk, normalsQtySpin, normalsLenSpin, openLastChk, filterOutlierChk, outlierAlphaSpin]() {
        QSettings settings;
        settings.setValue(QStringLiteral("Preferences/autoZoom"), autoZoomChk->isChecked());
        settings.setValue(QStringLiteral("Preferences/defaultNormalsQty"), normalsQtySpin->value());
        settings.setValue(QStringLiteral("Preferences/defaultNormalsLen"), normalsLenSpin->value());
        settings.setValue(QStringLiteral("Preferences/openLastProjectAutomatically"), openLastChk->isChecked());
        settings.setValue(QStringLiteral("Preferences/filterOutlierPoints"), filterOutlierChk->isChecked());
        settings.setValue(QStringLiteral("Preferences/outlierAlphaDegrees"), outlierAlphaSpin->value());
        accept();
    });

    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

} // namespace ExpressDesigner