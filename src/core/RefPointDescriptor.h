#pragma once
#include <QString>
#include <QPointF>
#include <QVariant>
#include <QMetaType>

namespace ExpressDesigner {

class CustomObject;

struct RefPointDescriptor {
    enum Kind { PointObject = 0, CurveBegin = 1, CurveEnd = 2 };

    Kind kind = PointObject;
    CustomObject* sourceObj = nullptr;

    QPointF resolve() const;
    QString displayName() const;

    bool isValid() const { return sourceObj != nullptr; }
};

} // namespace ExpressDesigner

Q_DECLARE_METATYPE(ExpressDesigner::RefPointDescriptor)