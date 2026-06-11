#pragma once
#include <QVector>
#include <QPointF>

namespace ExpressDesigner {
namespace Optics {

class FresnelLensCalculator {
public:
    struct Input {
        QVector<QPointF> inputWF;
        double inputIndex = 1.0;
        QVector<QPointF> outputWF;
        double outputIndex = 1.5;
        QPointF referencePoint;
        int numSegments = 20;
    };
    struct Result {
        QVector<QVector<QPointF>> lensParts;
        bool success = false;
    };
    static Result calculate(const Input& input);
};

} // namespace Optics
} // namespace ExpressDesigner