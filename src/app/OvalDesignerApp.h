#pragma once

#include <QObject>
#include <QString>

class QMainWindow;

namespace ExpressDesigner {

class Project;
class MainWindow;

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

    MainWindow* mainWindow() const;

signals:
    void projectOpened(Project* project);
    void projectClosed();

private:
    Project* m_project = nullptr;
    MainWindow* m_mainWindow = nullptr;
};

} // namespace ExpressDesigner