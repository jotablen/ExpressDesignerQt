#pragma once
#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>
#include <core/Project.h>
#include <core/CustomObject.h>

namespace ExpressDesigner {

class CopyObjectDialog : public QDialog {
    Q_OBJECT
public:
    explicit CopyObjectDialog(QWidget* parent = nullptr);

    void setProject(Project* project);
    QString sourceName() const;
    QString newName() const;

private:
    QComboBox* m_objectCombo;
    QLineEdit* m_newNameEdit;
};

} // namespace ExpressDesigner