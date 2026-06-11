#include "NumericUtils.h"
#include <QtMath>
#include <algorithm>

namespace ExpressDesigner {
namespace Utils {

void formatValue(double& value, int significantDigits)
{
    if (qAbs(value) < 1e-15) { value = 0.0; return; }
    double factor = qPow(10.0, significantDigits - 1 - static_cast<int>(qFloor(qLn(qAbs(value)) / qLn(10.0))));
    value = qRound(value * factor) / factor;
}

void formatPoint(QPointF& point, int significantDigits)
{
    formatValue(point.rx(), significantDigits);
    formatValue(point.ry(), significantDigits);
}

void formatPointList(QVector<QPointF>& points, int significantDigits)
{
    for (auto& p : points) formatPoint(p, significantDigits);
}

bool isNearlyEqual(double a, double b, double epsilon)
{
    return qAbs(a - b) <= epsilon;
}

double iterativeSolver(double initial, double target, double factor, int maxIterations)
{
    double current = initial;
    double direction = 1.0;
    for (int i = 0; i < maxIterations; ++i) {
        double error = target - current;
        if (qAbs(error) < 1e-12) break;
        current += direction * error * factor;
        if (i > 0 && qAbs(error) > qAbs(target - current)) direction = -direction;
    }
    return current;
}

} // namespace Utils
} // namespace ExpressDesigner