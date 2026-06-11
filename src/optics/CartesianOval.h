#pragma once

#include <QVector>
#include <QPointF>
#include <QString>
#include <QPointF>

namespace ExpressDesigner {
namespace Optics {

class CartesianOvalCalculator {
public:
    struct Input {
        QVector<QPointF> wavefront1;
        double index1 = 1.0;
        bool isWF1Real = true;
        QVector<QPointF> wavefront2;
        double index2 = 1.0;
        bool isWF2Real = true;
        QPointF referencePoint;
        int numResultPoints = 100;
    };

    struct Result {
        QVector<QPointF> ovalPoints;
        double opticalPathLength = 0.0;
        bool success = false;
        QString errorMessage;
    };

    static Result calculate(const Input& input);
};

} // namespace Optics
} // namespace ExpressDesigner