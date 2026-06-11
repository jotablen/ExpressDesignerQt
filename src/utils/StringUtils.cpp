#include "StringUtils.h"

namespace ExpressDesigner {
namespace Utils {

QString pointToString(const QPointF& point, int precision)
{
    return QStringLiteral("(%1, %2)")
        .arg(point.x(), 0, 'f', precision)
        .arg(point.y(), 0, 'f', precision);
}

QString pointListToString(const QVector<QPointF>& points, int precision)
{
    QStringList list;
    for (const auto& p : points) list << pointToString(p, precision);
    return list.join(QStringLiteral(", "));
}

QString doubleToString(double value, int precision)
{
    return QString::number(value, 'f', precision);
}

} // namespace Utils
} // namespace ExpressDesigner