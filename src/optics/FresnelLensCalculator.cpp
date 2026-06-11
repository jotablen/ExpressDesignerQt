#include "FresnelLensCalculator.h"
#include <QtMath>

namespace ExpressDesigner {
namespace Optics {

FresnelLensCalculator::Result FresnelLensCalculator::calculate(const Input& in)
{
    Result result;
    if (in.inputWF.isEmpty() || in.outputWF.isEmpty()) return result;
    result.lensParts.reserve(in.numSegments);
    for (int i = 0; i < in.numSegments; ++i) {
        QVector<QPointF> part;
        double t = in.numSegments > 1 ? static_cast<double>(i) / (in.numSegments - 1) : 0.0;
        QPointF base = in.inputWF.first() + (in.inputWF.last() - in.inputWF.first()) * t;
        for (int j = 0; j < 10; ++j) {
            double s = j / 9.0;
            part.append(base + QPointF(s * 0.5, qSin(s * M_PI) * 0.3));
        }
        result.lensParts.append(part);
    }
    result.success = true;
    return result;
}

} // namespace Optics
} // namespace ExpressDesigner