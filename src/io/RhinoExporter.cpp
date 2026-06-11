#include "RhinoExporter.h"
#include "CustomObject.h"
#include "Project.h"
#include <QFile>
#include <QTextStream>

namespace ExpressDesigner {

bool RhinoExporter::exportObject(const CustomObject* obj, const QString& filePath, bool exchangeYZ)
{
    if (!obj) return false;
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QTextStream stream(&file);
    stream << obj->toRhinoScript(exchangeYZ) << "\n";
    file.close();
    return true;
}

bool RhinoExporter::exportAll(const Project* project, const QString& filePath, bool includeWFs, bool exchangeYZ)
{
    if (!project) return false;
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QTextStream stream(&file);
    stream << "! ExpressDesigner RhinoScript Export\n";
    stream << "! Project: " << project->name() << "\n\n";
    for (const auto* obj : project->dataObjects()) {
        if (!includeWFs && obj->isWavefront()) continue;
        stream << obj->toRhinoScript(exchangeYZ) << "\n";
    }
    for (const auto* obj : project->resultObjects()) {
        stream << obj->toRhinoScript(exchangeYZ) << "\n";
    }
    file.close();
    return true;
}

} // namespace ExpressDesigner