#include "LineObject.h"
#include <QtMath>

namespace ExpressDesigner {

LineObject::LineObject(const QString& name, bool isWF, QObject* parent)
    : CustomObject(isWF ? withWavefront(ObjectType::Line) : ObjectType::Line, name, parent)
{
    m_controlPoints.resize(2);
}

LineObject::~LineObject() = default;

QPointF LineObject::startPoint() const
{
    return m_controlPoints.value(0, QPointF());
}

QPointF LineObject::endPoint() const
{
    return m_controlPoints.value(1, QPointF());
}

void LineObject::setStartPoint(const QPointF& pt)
{
    if (m_controlPoints.size() < 2) m_controlPoints.resize(2);
    m_controlPoints[0] = pt;
    emit controlPointsChanged();
}

void LineObject::setEndPoint(const QPointF& pt)
{
    if (m_controlPoints.size() < 2) m_controlPoints.resize(2);
    m_controlPoints[1] = pt;
    emit controlPointsChanged();
}

double LineObject::length() const
{
    auto d = endPoint() - startPoint();
    return qSqrt(d.x() * d.x() + d.y() * d.y());
}

void LineObject::saveToJson(QJsonObject& json) const
{
    CustomObject::saveToJson(json);
    json[QStringLiteral("objectSubType")] = QStringLiteral("line");
}

void LineObject::loadFromJson(const QJsonObject& json)
{
    CustomObject::loadFromJson(json);
}

QString LineObject::toRhinoScript(bool exchangeYZ) const
{
    QPointF p1 = startPoint();
    QPointF p2 = endPoint();
    return QStringLiteral("_Line %1,%2,0 %3,%4,0")
        .arg(p1.x(), 0, 'f', 8).arg(p1.y(), 0, 'f', 8)
        .arg(p2.x(), 0, 'f', 8).arg(p2.y(), 0, 'f', 8);
}

LineObject* LineObject::clone(QObject* parent) const
{
    auto* obj = new LineObject(m_name, isWavefront(), parent);
    obj->m_uuid = m_uuid;
    obj->m_refractiveIndex = m_refractiveIndex;
    obj->m_controlPoints = m_controlPoints;
    obj->m_referencePoint = m_referencePoint;
    obj->m_visible = m_visible;
    obj->m_showNormals = m_showNormals;
    obj->m_normalFlipped = m_normalFlipped;
    return obj;
}

} // namespace ExpressDesigner