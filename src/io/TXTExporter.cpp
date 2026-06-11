#include "TXTExporter.h"
#include "CustomObject.h"
#include <QFile>
#include <QTextStream>

namespace ExpressDesigner {

bool TXTExporter::exportToFile(const CustomObject* obj, const QString& filePath)
{
    if (!obj) return false;
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QTextStream stream(&file);
    stream << obj->name() << "\n";
    for (const auto& pt : obj->controlPoints()) {
        stream << pt.x() << " " << pt.y() << "\n";
    }
    file.close();
    return true;
}

} // namespace ExpressDesigner