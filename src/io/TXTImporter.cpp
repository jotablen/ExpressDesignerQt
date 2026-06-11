#include "TXTImporter.h"
#include "CustomObject.h"
#include <QFile>
#include <QTextStream>

namespace ExpressDesigner {

CustomObject* TXTImporter::importFromFile(const QString& filePath, QObject* parent)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return nullptr;
    QTextStream stream(&file);
    QString name = stream.readLine().trimmed();
    auto* obj = new CustomObject(ObjectType::Curve, name, parent);
    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        if (line.isEmpty()) continue;
        auto parts = line.split(' ', Qt::SkipEmptyParts);
        if (parts.size() >= 2) {
            obj->addControlPoint(QPointF(parts[0].toDouble(), parts[1].toDouble()));
        }
    }
    file.close();
    return obj;
}

} // namespace ExpressDesigner