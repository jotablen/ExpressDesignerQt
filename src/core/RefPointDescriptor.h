#pragma once
#include <QString>
#include <QPointF>

namespace ExpressDesigner {

class CustomObject;

struct RefPointDescriptor {
    enum Kind { PointObject = 0, CurveBegin = 1, CurveEnd = 2 };

    Kind kind = PointObject;
    CustomObject* sourceObj = nullptr;
    QString sourceName; // stored for re-resolution when pointer is stale

    QPointF resolve() const;
    QString displayName() const;

    bool isValid() const { return sourceObj != nullptr || !sourceName.isEmpty(); }
};

} // namespace ExpressDesigner
