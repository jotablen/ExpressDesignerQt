#pragma once
#include <QString>

namespace ExpressDesigner {
class CustomObject;

class TXTImporter {
public:
    static CustomObject* importFromFile(const QString& filePath, QObject* parent = nullptr);
};
} // namespace ExpressDesigner