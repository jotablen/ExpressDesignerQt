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
    switch (kind) {
    case CurveBegin: return sourceObj->name() + QStringLiteral(" (Begin)");
    case CurveEnd:   return sourceObj->name() + QStringLiteral(" (End)");
    case PointObject:
    default:         return sourceObj->name();
    }
}

} // namespace ExpressDesigner