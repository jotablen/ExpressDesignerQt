#include "RefPointDescriptor.h"
#include "CustomObject.h"

namespace ExpressDesigner {

QPointF RefPointDescriptor::resolve() const
{
    if (!sourceObj) return QPointF();
    const auto& pts = sourceObj->controlPoints();
    if (pts.isEmpty()) return QPointF();

    switch (kind) {
    case CurveBegin: return pts.first();
    case CurveEnd:   return pts.last();
    case PointObject:
    default:         return pts.first();
    }
}

QString RefPointDescriptor::displayName() const
{
    if (!sourceObj) return QStringLiteral("(none)");
    QPointF pt = resolve();
    QString coordStr = QStringLiteral(" (%1, %2)")
        .arg(pt.x(), 0, 'f', 3)
        .arg(pt.y(), 0, 'f', 3);
    switch (kind) {
    case CurveBegin: return sourceObj->name() + QStringLiteral(" (Begin)") + coordStr;
    case CurveEnd:   return sourceObj->name() + QStringLiteral(" (End)") + coordStr;
    case PointObject:
    default:         return sourceObj->name() + coordStr;
    }
}

} // namespace ExpressDesigner
