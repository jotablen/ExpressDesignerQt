#pragma once

#include "CustomObject.h"

namespace ExpressDesigner {

class ArcObject : public CustomObject {
    Q_OBJECT

public:
    explicit ArcObject(const QString& name = QString(), bool isWF = false,
                       QObject* parent = nullptr);
    ~ArcObject() override;

    QPointF center() const;
    void setCenter(const QPointF& center);

    double radius() const;
    void setRadius(double r);

    double startAngle() const;
    void setStartAngle(double angle);

    double endAngle() const;
    void setEndAngle(double angle);

    int numPoints() const;
    void setNumPoints(int n);

    QVector<QPointF> generateArcPoints() const;

    void saveToJson(QJsonObject& json) const override;
    void loadFromJson(const QJsonObject& json) override;
    QString toRhinoScript(bool exchangeYZ = false) const override;
    ArcObject* clone(QObject* parent = nullptr) const;

private:
    QPointF m_center;
    double m_radius = 1.0;
    double m_startAngle = 0.0;
    double m_endAngle = 90.0;
    int m_numPoints = 50;
};

} // namespace ExpressDesigner