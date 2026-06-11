#pragma once
#include <QString>

namespace ExpressDesigner {
class CustomObject;

class TXTExporter {
public:
    static bool exportToFile(const CustomObject* obj, const QString& filePath);
};

} // namespace ExpressDesigner