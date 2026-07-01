#pragma once

#include "BaseObject.h"
#include <QVector>
#include <QPointF>
#include <QColor>
#include <QPair>

namespace ExpressDesigner {

/**
 * @brief Geometric object with visual representation on chart.
 *
 * Holds control points, refractive index, reference point,
 * and provides methods for chart rendering, normal computation,
 * and Rhino script export.
 *
 * Equivalent to TLPE_CustomObject (inherits TLPE_BaseObject).
 */
class CustomObject : public BaseObject {
    Q_OBJECT
    Q_PROPERTY(double refractiveIndex READ refractiveIndex WRITE setRefractiveIndex NOTIFY refractiveIndexChanged)

public:
    explicit CustomObject(ObjectType type, const QString& name = QString(),
                          QObject* parent = nullptr);
    ~CustomObject() override;

    // --- Refractive Index ---
    void setRefractiveIndex(double index);
    double refractiveIndex() const;

    // --- Control Points ---
    const QVector<QPointF>& controlPoints() const;
    void setControlPoints(const QVector<QPointF>& points);
    void addControlPoint(const QPointF& point);
    void removeControlPoint(int index);
    void updateControlPoint(int index, const QPointF& point);
    int controlPointCount() const;
    void clearControlPoints();

    // --- Reference Point ---
    void setReferencePoint(const QPointF& point);
    QPointF referencePoint() const;

    // --- Selected Point ---
    void setSelectedPointIndex(int index);
    int selectedPointIndex() const;
    QPointF selectedPoint() const;

    // --- Normals ---
    QVector<QPair<QPointF, QPointF>> computeNormals() const;
    QVector<QPair<QPointF, QPointF>> computeNormals(int numPoints) const;
    QVector<QPair<QPointF, QPointF>> computeNormals(int numPoints, double length) const;

    // --- Chart Colors ---
    void setNormalColor(const QColor& color);
    QColor normalColor() const;

    void setSelectedColor(const QColor& color);
    QColor selectedColor() const;

    // --- Serialization ---
    void saveToJson(QJsonObject& json) const override;
    void loadFromJson(const QJsonObject& json) override;

    // --- Export ---
    virtual QString toRhinoScript(bool exchangeYZ = false) const;
    virtual QString toTxtFormat() const;

    // --- Clone ---
    virtual CustomObject* clone(QObject* parent = nullptr) const;

signals:
    void refractiveIndexChanged(double index);
    void controlPointsChanged();
    void referencePointChanged(const QPointF& point);

protected:
    double m_refractiveIndex = 1.0;
    QVector<QPointF> m_controlPoints;
    QPointF m_referencePoint = QPointF(0.0, 0.0);
    int m_selectedPointIndex = -1;

    QColor m_normalColor = QColor(0, 0, 255);
    QColor m_selectedColor = QColor(255, 0, 0);

    // Helper for Rhino export of control points
    QString pointsToRhinoFormat(const QVector<QPointF>& pts, bool exchangeYZ = false) const;
};

} // namespace ExpressDesigner