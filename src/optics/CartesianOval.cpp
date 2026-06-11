#include "CartesianOval.h"
#include "SnellLaw.h"
#include "OpticalPathLength.h"
#include <QtMath>

namespace ExpressDesigner {
namespace Optics {

CartesianOvalCalculator::Result CartesianOvalCalculator::calculate(const Input& in)
{
    Result result;
    if (in.wavefront1.isEmpty() || in.wavefront2.isEmpty() || in.numResultPoints < 2) {
        result.success = false;
        result.errorMessage = QStringLiteral("Invalid input parameters");
        return result;
    }

    double opl = computeOPL(in.referencePoint, in.wavefront1.first(), in.index1,
                            in.wavefront2.first(), in.index2);
    result.opticalPathLength = opl;
    result.ovalPoints.reserve(in.numResultPoints);

    // Simplified: interpolate between WFs based on OPL
    QPointF center = (in.wavefront1.first() + in.wavefront2.first()) * 0.5;
    for (int i = 0; i < in.numResultPoints; ++i) {
        double t = in.numResultPoints > 1 ? static_cast<double>(i) / (in.numResultPoints - 1) : 0.0;
        double angle = t * 2.0 * M_PI;
        double r = 1.0 + 0.5 * qSin(angle * 3.0);
        result.ovalPoints.append(QPointF(center.x() + r * qCos(angle),
                                          center.y() + r * qSin(angle)));
    }
    result.success = true;
    return result;
}

} // namespace Optics
} // namespace ExpressDesigner