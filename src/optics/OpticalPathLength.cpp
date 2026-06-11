#include "OpticalPathLength.h"
#include <QtMath>

namespace ExpressDesigner {
namespace Optics {

double computeOPL(const QPointF& refPoint,
                  const QPointF& wf1Point, double idx1,
                  const QPointF& wf2Point, double idx2)
{
    auto d1 = wf1Point - refPoint;
    auto d2 = wf2Point - refPoint;
    double dist1 = qSqrt(d1.x() * d1.x() + d1.y() * d1.y());
    double dist2 = qSqrt(d2.x() * d2.x() + d2.y() * d2.y());
    return idx1 * dist1 + idx2 * dist2;
}

double computeOpticalPath(const QVector<QPointF>& path, const QVector<double>& indices)
{
    if (path.size() < 2) return 0.0;
    double total = 0.0;
    for (int i = 0; i < path.size() - 1; ++i) {
        auto d = path[i + 1] - path[i];
        double dist = qSqrt(d.x() * d.x() + d.y() * d.y());
        double idx = (i < indices.size()) ? indices[i] : 1.0;
        total += idx * dist;
    }
    return total;
}

} // namespace Optics
} // namespace ExpressDesigner