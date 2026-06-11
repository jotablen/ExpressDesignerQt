#pragma once
#include <QPointF>
#include <QVector>

namespace ExpressDesigner {
namespace Utils {

void formatValue(double& value, int significantDigits);
void formatPoint(QPointF& point, int significantDigits);
void formatPointList(QVector<QPointF>& points, int significantDigits);
bool isNearlyEqual(double a, double b, double epsilon = 1e-12);
double iterativeSolver(double initial, double target, double factor, int maxIterations = 100);

} // namespace Utils
} // namespace ExpressDesigner