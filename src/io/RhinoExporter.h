#pragma once
#include <QString>

namespace ExpressDesigner {
class CustomObject;
class Project;

class RhinoExporter {
public:
    static bool exportObject(const CustomObject* obj, const QString& filePath, bool exchangeYZ = false);
    static bool exportAll(const Project* project, const QString& filePath, bool includeWFs = true, bool exchangeYZ = false);
};
} // namespace ExpressDesigner