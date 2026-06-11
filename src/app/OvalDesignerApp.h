#pragma once

#include <QObject>
#include <QString>

namespace ExpressDesigner {

class Project;

class ExpressDesignerApp : public QObject {
    Q_OBJECT
public:
    explicit ExpressDesignerApp(QObject* parent = nullptr);
    ~ExpressDesignerApp() override;

    void initialize();
    void openProject(const QString& filePath);
    void newProject();
    void closeProject();

    Project* currentProject() const;
    bool hasProject() const;

signals:
    void projectOpened(Project* project);
    void projectClosed();

private:
    Project* m_project = nullptr;
};

} // namespace ExpressDesigner