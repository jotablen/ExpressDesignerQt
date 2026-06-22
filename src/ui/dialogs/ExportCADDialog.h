#pragma once
#include <QDialog>
#include <QListWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <core/Project.h>

namespace ExpressDesigner {

class ExportCADDialog : public QDialog {
    Q_OBJECT
public:
    explicit ExportCADDialog(QWidget* parent = nullptr);
    void setProject(Project* project);

    // Selected objects
    QStringList selectedObjectNames() const;

    // Common
    QString fileName() const { return m_fileNameEdit->text(); }
    bool wiresOnly() const { return m_wiresOnlyCheck->isChecked(); }

    // Rotational extrusion
    bool rotationalEnabled() const { return m_rotationalCheck->isChecked(); }
    QString rotationalAxis() const;
    double rotationalAngleStart() const { return m_rotAngleStartSpin->value(); }
    double rotationalAngleEnd() const { return m_rotAngleEndSpin->value(); }
    int rotationalAngularSteps() const { return m_rotAngularStepsSpin->value(); }

    // Linear extrusion
    bool linearEnabled() const { return m_linearCheck->isChecked(); }
    QString linearDirection() const;
    double linearWideness() const { return m_linearWidenessSpin->value(); }

private slots:
    void onBrowse();
    void onSelectionChanged();
    void onIncludeWFsToggled(bool checked);
    void onRotationalToggled(bool checked);
    void onLinearToggled(bool checked);

private:
    void populateObjects(Project* project);
    void refilterObjects();
    void updateExportButton();

    Project* m_project = nullptr;

    // Left panel — object list
    QListWidget* m_objectList = nullptr;
    QCheckBox* m_includeWFsCheck = nullptr;

    // Right panel — options
    QCheckBox* m_wiresOnlyCheck = nullptr;
    QLineEdit* m_fileNameEdit = nullptr;

    // Rotational extrusion group
    QCheckBox* m_rotationalCheck = nullptr;
    QComboBox* m_rotAxisCombo = nullptr;
    QDoubleSpinBox* m_rotAngleStartSpin = nullptr;
    QDoubleSpinBox* m_rotAngleEndSpin = nullptr;
    QDoubleSpinBox* m_rotAngularStepsSpin = nullptr;

    // Linear extrusion group
    QCheckBox* m_linearCheck = nullptr;
    QComboBox* m_linearDirCombo = nullptr;
    QDoubleSpinBox* m_linearWidenessSpin = nullptr;

    // Buttons
    QPushButton* m_exportButton = nullptr;
};

} // namespace ExpressDesigner