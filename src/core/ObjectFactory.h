#pragma once

#include "ObjectTypes.h"
#include <QJsonObject>

namespace ExpressDesigner {

class CustomObject;
class CustomOperation;

class ObjectFactory {
public:
    static CustomObject* createFromType(ObjectType type, const QString& name,
                                        bool isWF, QObject* parent = nullptr);
    static CustomObject* createFromJson(const QJsonObject& json, QObject* parent = nullptr);

    static CustomOperation* createOperation(OperationType type, const QString& name,
                                            QObject* parent = nullptr);
    static CustomOperation* createOperationFromJson(const QJsonObject& json,
                                                    QObject* parent = nullptr);
};

} // namespace ExpressDesigner