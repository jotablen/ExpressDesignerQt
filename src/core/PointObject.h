#pragma once

#include "CustomObject.h"

namespace ExpressDesigner {

/**
 * @brief Point geometric object.
 *
 * Represents a single point in 2D space.
 * When isWavefront() is true, acts as a wavefront point with normals.
 *
 * Equivalent to TLPE_PointObj.
 */
class PointObject : public CustomObject {
    Q_OBJECT

public:
    explicit PointObject(const QString& name = QString(), bool isWF = false,
                         QObject* parent = nullptr);
    ~PointObject() override;

    // Position (uses first control point or reference point)
    QPointF position() const;
    void setPosition(const QPointF& pos);

    // Radius for point display
    void setRadius(double radius);
    double radius() const;

    // Serialization
    void saveToJson(QJsonObject& json) const override;
    void loadFromJson(const QJsonObject& json) override;

    // Export
    QString toRhinoScript(bool exchangeYZ = false) const override;

    // Clone
    PointObject* clone(QObject* parent = nullptr) const;

private:
    double m_radius = 0.3;
};

} // namespace ExpressDesigner