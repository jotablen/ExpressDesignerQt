#include "PointObject.h"

namespace ExpressDesigner {

PointObject::PointObject(const QString& name, bool isWF, QObject* parent)
    : CustomObject(isWF ? withWavefront(ObjectType::Point) : ObjectType::Point, name, parent)
{
}

PointObject::~PointObject() = default;

QPointF PointObject::position() const
{
    if (m_controlPoints.isEmpty())
        return m_referencePoint;
    return m_controlPoints.first();
}

void PointObject::setPosition(const QPointF& pos)
{
    if (m_controlPoints.isEmpty())
        m_controlPoints.append(pos);
    else
        m_controlPoints[0] = pos;
    setReferencePoint(pos);
    emit controlPointsChanged();
    emit objectModified(QStringLiteral("position"));
}

void PointObject::setRadius(double radius) { m_radius = radius; }
double PointObject::radius() const { return m_radius; }

void PointObject::saveToJson(QJsonObject& json) const
{
    CustomObject::saveToJson(json);
    json[QStringLiteral("objectSubType")] = QStringLiteral("point");
    json[QStringLiteral("radius")] = m_radius;
}

void PointObject::loadFromJson(const QJsonObject& json)
{
    CustomObject::loadFromJson(json);
    m_radius = json[QStringLiteral("radius")].toDouble(0.3);
}

QString PointObject::toRhinoScript(bool exchangeYZ) const
{
    QPointF p = position();
    return QStringLiteral("_Point %1,%2,0").arg(p.x(), 0, 'f', 8).arg(p.y(), 0, 'f', 8);
}

PointObject* PointObject::clone(QObject* parent) const
{
    auto* obj = new PointObject(m_name, isWavefront(), parent);
    obj->m_uuid = m_uuid;
    obj->m_refractiveIndex = m_refractiveIndex;
    obj->m_controlPoints = m_controlPoints;
    obj->m_referencePoint = m_referencePoint;
    obj->m_radius = m_radius;
    obj->m_visible = m_visible;
    obj->m_showNormals = m_showNormals;
    obj->m_normalFlipped = m_normalFlipped;
    return obj;
}

} // namespace ExpressDesigner