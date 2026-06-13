#include "ArcObject.h"
#include <QtMath>

namespace ExpressDesigner {

ArcObject::ArcObject(const QString& name, bool isWF, QObject* parent)
    : CustomObject(isWF ? withWavefront(ObjectType::Arc) : ObjectType::Arc, name, parent)
{
}

ArcObject::~ArcObject() = default;

QPointF ArcObject::center() const { return m_center; }
void ArcObject::setCenter(const QPointF& center) { m_center = center; }

double ArcObject::radius() const { return m_radius; }
void ArcObject::setRadius(double r) { m_radius = r; }

double ArcObject::startAngle() const { return m_startAngle; }
void ArcObject::setStartAngle(double a) { m_startAngle = a; }

double ArcObject::endAngle() const { return m_endAngle; }
void ArcObject::setEndAngle(double a) { m_endAngle = a; }

int ArcObject::numPoints() const { return m_numPoints; }
void ArcObject::setNumPoints(int n) { m_numPoints = qMax(n, 3); }

QVector<QPointF> ArcObject::generateArcPoints() const
{
    QVector<QPointF> points;
    points.reserve(m_numPoints);
    double range = m_endAngle - m_startAngle;
    double radStart = qDegreesToRadians(m_startAngle);
    for (int i = 0; i < m_numPoints; ++i) {
        double t = static_cast<double>(i) / (m_numPoints - 1);
        double angle = radStart + qDegreesToRadians(range * t);
        points.append(QPointF(
            m_center.x() + m_radius * qCos(angle),
            m_center.y() + m_radius * qSin(angle)
        ));
    }
    return points;
}

void ArcObject::saveToJson(QJsonObject& json) const
{
    CustomObject::saveToJson(json);
    json[QStringLiteral("objectSubType")] = QStringLiteral("arc");
    QJsonObject centerObj;
    centerObj[QStringLiteral("x")] = m_center.x();
    centerObj[QStringLiteral("y")] = m_center.y();
    json[QStringLiteral("center")] = centerObj;
    json[QStringLiteral("radius")] = m_radius;
    json[QStringLiteral("startAngle")] = m_startAngle;
    json[QStringLiteral("endAngle")] = m_endAngle;
    json[QStringLiteral("numPoints")] = m_numPoints;
}

void ArcObject::loadFromJson(const QJsonObject& json)
{
    CustomObject::loadFromJson(json);
    QJsonObject c = json[QStringLiteral("center")].toObject();
    m_center = QPointF(c[QStringLiteral("x")].toDouble(), c[QStringLiteral("y")].toDouble());
    m_radius = json[QStringLiteral("radius")].toDouble(1.0);
    m_startAngle = json[QStringLiteral("startAngle")].toDouble(0.0);
    m_endAngle = json[QStringLiteral("endAngle")].toDouble(90.0);
    m_numPoints = json[QStringLiteral("numPoints")].toInt(50);
    setControlPoints(generateArcPoints());
}

QString ArcObject::toRhinoScript(bool exchangeYZ) const
{
    QPointF c = m_center;
    return QStringLiteral("_Arc %1,%2,0 %3 %4 %5")
        .arg(c.x(), 0, 'f', 8).arg(c.y(), 0, 'f', 8)
        .arg(m_radius, 0, 'f', 8)
        .arg(m_startAngle, 0, 'f', 8)
        .arg(m_endAngle, 0, 'f', 8);
}

ArcObject* ArcObject::clone(QObject* parent) const
{
    auto* obj = new ArcObject(m_name, isWavefront(), parent);
    obj->m_uuid = m_uuid;
    obj->m_refractiveIndex = m_refractiveIndex;
    obj->m_controlPoints = m_controlPoints;
    obj->m_referencePoint = m_referencePoint;
    obj->m_center = m_center;
    obj->m_radius = m_radius;
    obj->m_startAngle = m_startAngle;
    obj->m_endAngle = m_endAngle;
    obj->m_numPoints = m_numPoints;
    obj->m_visible = m_visible;
    obj->m_showNormals = m_showNormals;
    return obj;
}

} // namespace ExpressDesigner