#include "CurveObject.h"

namespace ExpressDesigner {

CurveObject::CurveObject(const QString& name, bool isWF, QObject* parent)
    : CustomObject(isWF ? withWavefront(ObjectType::Curve) : ObjectType::Curve, name, parent)
{
}

CurveObject::~CurveObject() = default;

int CurveObject::maxPoints() const { return m_maxPoints; }
void CurveObject::setMaxPoints(int n) { m_maxPoints = qMax(n, 2); }

int CurveObject::splineOrder() const { return m_splineOrder; }
void CurveObject::setSplineOrder(int order) { m_splineOrder = qMax(order, 2); }

bool CurveObject::isOpen() const { return m_isOpen; }
void CurveObject::setOpen(bool open) { m_isOpen = open; }

QVector<QPointF> CurveObject::discretize(int numPoints) const
{
    if (m_controlPoints.isEmpty()) return {};
    // Simple linear interpolation between control points
    QVector<QPointF> result;
    result.reserve(numPoints);
    int n = m_controlPoints.size();
    for (int i = 0; i < numPoints; ++i) {
        double t = static_cast<double>(i) / (numPoints - 1);
        double seg = t * (n - 1);
        int idx = qMin(static_cast<int>(seg), n - 2);
        double lt = seg - idx;
        QPointF pt = m_controlPoints[idx] + (m_controlPoints[idx + 1] - m_controlPoints[idx]) * lt;
        result.append(pt);
    }
    return result;
}

void CurveObject::saveToJson(QJsonObject& json) const
{
    CustomObject::saveToJson(json);
    json[QStringLiteral("objectSubType")] = QStringLiteral("curve");
    json[QStringLiteral("splineOrder")] = m_splineOrder;
    json[QStringLiteral("isOpen")] = m_isOpen;
}

void CurveObject::loadFromJson(const QJsonObject& json)
{
    CustomObject::loadFromJson(json);
    m_splineOrder = json[QStringLiteral("splineOrder")].toInt(3);
    m_isOpen = json[QStringLiteral("isOpen")].toBool(true);
}

QString CurveObject::toRhinoScript(bool exchangeYZ) const
{
    return pointsToRhinoFormat(m_controlPoints, exchangeYZ);
}

CurveObject* CurveObject::clone(QObject* parent) const
{
    auto* obj = new CurveObject(m_name, isWavefront(), parent);
    obj->m_uuid = m_uuid;
    obj->m_refractiveIndex = m_refractiveIndex;
    obj->m_controlPoints = m_controlPoints;
    obj->m_referencePoint = m_referencePoint;
    obj->m_splineOrder = m_splineOrder;
    obj->m_isOpen = m_isOpen;
    obj->m_visible = m_visible;
    obj->m_showNormals = m_showNormals;
    obj->m_normalFlipped = m_normalFlipped;
    return obj;
}

} // namespace ExpressDesigner