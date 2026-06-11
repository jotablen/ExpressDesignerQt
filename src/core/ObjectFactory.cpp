#include "ObjectFactory.h"
#include "PointObject.h"
#include "LineObject.h"
#include "ArcObject.h"
#include "CurveObject.h"
#include "CarthesianOvalOperation.h"
#include "PropagateWFOperation.h"

namespace ExpressDesigner {

CustomObject* ObjectFactory::createFromType(ObjectType type, const QString& name,
                                            bool isWF, QObject* parent)
{
    uint16_t base = toBaseType(type);
    switch (base) {
    case 0x001: return new PointObject(name, isWF, parent);
    case 0x002: return new LineObject(name, isWF, parent);
    case 0x003: return new ArcObject(name, isWF, parent);
    case 0x004: return new CurveObject(name, isWF, parent);
    default:   return new CustomObject(type, name, parent);
    }
}

CustomObject* ObjectFactory::createFromJson(const QJsonObject& json, QObject* parent)
{
    QString subType = json.value(QStringLiteral("objectSubType")).toString();
    QString name = json.value(QStringLiteral("name")).toString();
    CustomObject* obj = nullptr;

    if (subType == QLatin1String("point"))
        obj = new PointObject(name, false, parent);
    else if (subType == QLatin1String("line"))
        obj = new LineObject(name, false, parent);
    else if (subType == QLatin1String("arc"))
        obj = new ArcObject(name, false, parent);
    else if (subType == QLatin1String("curve"))
        obj = new CurveObject(name, false, parent);
    else
        obj = new CustomObject(ObjectType::Curve, name, parent);

    if (obj)
        obj->loadFromJson(json);

    return obj;
}

CustomOperation* ObjectFactory::createOperation(OperationType type, const QString& name,
                                                QObject* parent)
{
    switch (type) {
    case OperationType::CartesianOval:
        return new CarthesianOvalOperation(name, parent);
    case OperationType::PropagateWF:
        return new PropagateWFOperation(name, parent);
    default:
        return new CustomOperation(type, name, parent);
    }
}

CustomOperation* ObjectFactory::createOperationFromJson(const QJsonObject& json,
                                                        QObject* parent)
{
    QString subType = json.value(QStringLiteral("operationSubType")).toString();
    QString name = json.value(QStringLiteral("name")).toString();
    CustomOperation* op = nullptr;

    if (subType == QLatin1String("cartesianOval"))
        op = new CarthesianOvalOperation(name, parent);
    else if (subType == QLatin1String("propagateWF"))
        op = new PropagateWFOperation(name, parent);
    else
        op = new CustomOperation(OperationType::Custom, name, parent);

    if (op)
        op->loadFromJson(json);

    return op;
}

} // namespace ExpressDesigner