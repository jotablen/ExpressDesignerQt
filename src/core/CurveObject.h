#pragma once

#include "CustomObject.h"

namespace ExpressDesigner {

class CurveObject : public CustomObject {
    Q_OBJECT

public:
    explicit CurveObject(const QString& name = QString(), bool isWF = false,
                         QObject* parent = nullptr);
    ~CurveObject() override;

    int maxPoints() const;
    void setMaxPoints(int n);

    int splineOrder() const;
    void setSplineOrder(int order);

    bool isOpen() const;
    void setOpen(bool open);

    QVector<QPointF> discretize(int numPoints) const;

    void saveToJson(QJsonObject& json) const override;
    void loadFromJson(const QJsonObject& json) override;
    QString toRhinoScript(bool exchangeYZ = false) const override;
    CurveObject* clone(QObject* parent = nullptr) const;

private:
    int m_maxPoints = 100;
    int m_splineOrder = 3;
    bool m_isOpen = true;
};

} // namespace ExpressDesigner