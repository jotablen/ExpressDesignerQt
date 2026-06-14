#pragma once
#include <QDialog>

class QCheckBox;
class QSpinBox;
class QDoubleSpinBox;

namespace ExpressDesigner {

class PreferencesDialog : public QDialog {
    Q_OBJECT
public:
    explicit PreferencesDialog(QWidget* parent = nullptr);
};

} // namespace ExpressDesigner