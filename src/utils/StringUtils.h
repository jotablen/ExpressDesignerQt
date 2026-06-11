#pragma once
#include <QString>
#include <QPointF>
#include <QVector>

namespace ExpressDesigner {
namespace Utils {

QString pointToString(const QPointF& point, int precision = 6);
QString pointListToString(const QVector<QPointF>& points, int precision = 6);
QString doubleToString(double value, int precision = 6);

} // namespace Utils
} // namespace ExpressDesigner