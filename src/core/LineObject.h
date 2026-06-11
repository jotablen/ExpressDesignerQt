#pragma once

#include "CustomObject.h"

namespace ExpressDesigner {

/**
 * @brief Line segment geometric object.
 *
 * Defined by two endpoint control points.
 * Equivalent to TLPE_LineObj.
 */
class LineObject : public CustomObject {
    Q_OBJECT

public:
    explicit LineObject(const QString& name = QString(), bool isWF = false,
                        QObject* parent = nullptr);
    ~LineObject() override;

    QPointF startPoint() const;
    QPointF endPoint() const;
    void setStartPoint(const QPointF& pt);
    void setEndPoint(const QPointF& pt);

    double length() const;

    // Serialization
    void saveToJson(QJsonObject& json) const override;
    void loadFromJson(const QJsonObject& json) override;

    QString toRhinoScript(bool exchangeYZ = false) const override;
    LineObject* clone(QObject* parent = nullptr) const;
};

} // namespace ExpressDesigner