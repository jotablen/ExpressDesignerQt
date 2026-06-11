#pragma once
#include <QVector>
#include <QPointF>
#include <QString>

namespace ExpressDesigner {
namespace Optics {

class WavefrontPropagator {
public:
    struct Input {
        QVector<QPointF> wavefront;
        double inputIndex = 1.0;
        QVector<QPointF> refractingSurface;
        double outputIndex = 1.5;
        double offset = 0.0;
        int numResultPoints = 100;
        bool excludeTIR = false;
    };
    struct Result {
        QVector<QPointF> propagatedWF;
        QVector<QPointF> intersectionPoints;
        bool success = false;
        QString errorMessage;
    };
    static Result calculate(const Input& input);
};

} // namespace Optics
} // namespace ExpressDesigner