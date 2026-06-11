#include "OvalDesignerApp.h"
#include "Project.h"
#include "ProjectSerializer.h"

namespace ExpressDesigner {

ExpressDesignerApp::ExpressDesignerApp(QObject* parent) : QObject(parent) {}
ExpressDesignerApp::~ExpressDesignerApp() { closeProject(); }

void ExpressDesignerApp::initialize() {}

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

Project* ExpressDesignerApp::currentProject() const { return m_project; }
bool ExpressDesignerApp::hasProject() const { return m_project != nullptr; }

} // namespace ExpressDesigner