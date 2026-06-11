#include "WavefrontPropagator.h"
#include "SnellLaw.h"
#include "OpticalPathLength.h"
#include <QtMath>

namespace ExpressDesigner {
namespace Optics {

WavefrontPropagator::Result WavefrontPropagator::calculate(const Input& in)
{
    Result result;
    if (in.wavefront.isEmpty() || in.refractingSurface.isEmpty()) {
        result.success = false;
        result.errorMessage = QStringLiteral("Empty input wavefront or surface");
        return result;
    }
    result.propagatedWF.reserve(in.numResultPoints);
    result.success = true;
    // Simplified propagation
    for (int i = 0; i < in.numResultPoints; ++i) {
        double t = in.numResultPoints > 1 ? static_cast<double>(i) / (in.numResultPoints - 1) : 0.0;
        QPointF src = in.wavefront.first() + (in.wavefront.last() - in.wavefront.first()) * t;
        QPointF srf = in.refractingSurface.first() + (in.refractingSurface.last() - in.refractingSurface.first()) * t;
        QPointF dir = srf - src;
        bool tir = false;
        QPointF outDir = getDeflectedVector(dir, srf, in.inputIndex, in.outputIndex, tir);
        if (tir && in.excludeTIR) continue;
        result.propagatedWF.append(srf + outDir * (1.0 + in.offset));
    }
    return result;
}

} // namespace Optics
} // namespace ExpressDesigner