#include "OvalDesignerApp.h"
#include "Project.h"
#include "ProjectSerializer.h"
#include "ui/MainWindow.h"

namespace ExpressDesigner {

ExpressDesignerApp::ExpressDesignerApp(QObject* parent) : QObject(parent) {}

ExpressDesignerApp::~ExpressDesignerApp()
{
    closeProject();
    delete m_mainWindow;
}

void ExpressDesignerApp::initialize()
{
    m_mainWindow = new MainWindow();
    m_mainWindow->show();
}

void ExpressDesignerApp::openProject(const QString& filePath)
{
    closeProject();
    m_project = ProjectSerializer::load(filePath, this);
    if (m_project) emit projectOpened(m_project);
}

void ExpressDesignerApp::newProject()
{
    closeProject();
    m_project = new Project(QStringLiteral("Unnamed Project"), this);
    emit projectOpened(m_project);
}

void ExpressDesignerApp::closeProject()
{
    if (m_project) {
        emit projectClosed();
        delete m_project;
        m_project = nullptr;
    }
}

MainWindow* ExpressDesignerApp::mainWindow() const { return m_mainWindow; }

Project* ExpressDesignerApp::currentProject() const { return m_project; }
bool ExpressDesignerApp::hasProject() const { return m_project != nullptr; }

} // namespace ExpressDesigner