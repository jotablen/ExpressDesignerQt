#pragma once
#include <QDialog>
#include <QListWidget>
#include <QLineEdit>
#include <core/Project.h>

namespace ExpressDesigner {

class TranslateObjectDialog : public QDialog {
    Q_OBJECT
public:
    explicit TranslateObjectDialog(QWidget* parent = nullptr);
    void setProject(Project* project);
    void setSelectedObject(CustomObject* obj);

    QListWidget* objectList() const { return m_objectList; }
    QLineEdit* deltaXEdit() const { return m_deltaXEdit; }
    QLineEdit* deltaYEdit() const { return m_deltaYEdit; }

private:
    QListWidget* m_objectList = nullptr;
    QLineEdit* m_deltaXEdit = nullptr;
    QLineEdit* m_deltaYEdit = nullptr;
};

} // namespace ExpressDesigner