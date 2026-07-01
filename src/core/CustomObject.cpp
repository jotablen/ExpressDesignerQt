#include "CustomObject.h"
#include <QJsonArray>
#include <QtMath>

namespace ExpressDesigner {

CustomObject::CustomObject(ObjectType type, const QString& name, QObject* parent)
    : BaseObject(type, name, parent)
{
}

CustomObject::~CustomObject() = default;

void CustomObject::setRefractiveIndex(double index)
{
    if (!qFuzzyCompare(m_refractiveIndex, index)) {
        m_refractiveIndex = index;
        emit refractiveIndexChanged(m_refractiveIndex);
        emit objectModified(QStringLiteral("refractiveIndex"));
    }
}

double CustomObject::refractiveIndex() const { return m_refractiveIndex; }

const QVector<QPointF>& CustomObject::controlPoints() const { return m_controlPoints; }

void CustomObject::setControlPoints(const QVector<QPointF>& points)
{
    m_controlPoints = points;
    m_selectedPointIndex = -1;
    emit controlPointsChanged();
    emit objectModified(QStringLiteral("controlPoints"));
}

void CustomObject::addControlPoint(const QPointF& point)
{
    m_controlPoints.append(point);
    emit controlPointsChanged();
    emit objectModified(QStringLiteral("controlPoints"));
}

void CustomObject::removeControlPoint(int index)
{
    if (index >= 0 && index < m_controlPoints.size()) {
        m_controlPoints.removeAt(index);
        if (m_selectedPointIndex >= m_controlPoints.size())
            m_selectedPointIndex = -1;
        emit controlPointsChanged();
        emit objectModified(QStringLiteral("controlPoints"));
    }
}

void CustomObject::updateControlPoint(int index, const QPointF& point)
{
    if (index >= 0 && index < m_controlPoints.size()) {
        m_controlPoints[index] = point;
        emit controlPointsChanged();
        emit objectModified(QStringLiteral("controlPoints"));
    }
}

int CustomObject::controlPointCount() const { return m_controlPoints.size(); }

void CustomObject::clearControlPoints()
{
    m_controlPoints.clear();
    m_selectedPointIndex = -1;
    emit controlPointsChanged();
    emit objectModified(QStringLiteral("controlPoints"));
}

void CustomObject::setReferencePoint(const QPointF& point)
{
    if (m_referencePoint != point) {
        m_referencePoint = point;
        emit referencePointChanged(m_referencePoint);
        emit objectModified(QStringLiteral("referencePoint"));
    }
}

QPointF CustomObject::referencePoint() const { return m_referencePoint; }

void CustomObject::setSelectedPointIndex(int index)
{
    if (index >= -1 && index < m_controlPoints.size()) {
        m_selectedPointIndex = index;
        emit objectModified(QStringLiteral("selectedPoint"));
    }
}

int CustomObject::selectedPointIndex() const { return m_selectedPointIndex; }

QPointF CustomObject::selectedPoint() const
{
    if (m_selectedPointIndex >= 0 && m_selectedPointIndex < m_controlPoints.size())
        return m_controlPoints[m_selectedPointIndex];
    return QPointF();
}

QVector<QPair<QPointF, QPointF>> CustomObject::computeNormals() const
{
    return computeNormals(m_raysQty);
}

QVector<QPair<QPointF, QPointF>> CustomObject::computeNormals(int numPoints) const
{
    return computeNormals(numPoints, m_raysLen);
}

QVector<QPair<QPointF, QPointF>> CustomObject::computeNormals(int numPoints, double length) const
{
    QVector<QPair<QPointF, QPointF>> result;
    if (m_controlPoints.size() < 2 || numPoints < 1)
        return result;

    // Discretize the curve into numPoints segments
    result.reserve(numPoints);
    int totalSegments = m_controlPoints.size() - 1;
    for (int i = 0; i < numPoints; ++i) {
        double t = static_cast<double>(i) / (numPoints - 1);
        double segFloat = t * totalSegments;
        int seg = qMin(static_cast<int>(segFloat), totalSegments - 1);
        double localT = segFloat - seg;

        QPointF p0 = m_controlPoints[seg];
        QPointF p1 = m_controlPoints[seg + 1];
        QPointF pos = p0 + (p1 - p0) * localT;

        // Compute normal (perpendicular to segment direction)
        QPointF dir = p1 - p0;
        double len = qSqrt(dir.x() * dir.x() + dir.y() * dir.y());
        if (len > 1e-12) {
            QPointF normal(-dir.y() / len, dir.x() / len);
            if (m_normalFlipped) normal = -normal;
            QPointF normalEnd = pos + normal * length;
            result.append({pos, normalEnd});
        }
    }
    return result;
}

void CustomObject::setNormalColor(const QColor& color) { m_normalColor = color; }
QColor CustomObject::normalColor() const { return m_normalColor; }

void CustomObject::setSelectedColor(const QColor& color) { m_selectedColor = color; }
QColor CustomObject::selectedColor() const { return m_selectedColor; }

void CustomObject::saveToJson(QJsonObject& json) const
{
    BaseObject::saveToJson(json);

    json[QStringLiteral("refractiveIndex")] = m_refractiveIndex;

    QJsonArray ptsArray;
    for (const auto& pt : m_controlPoints) {
        QJsonObject ptObj;
        ptObj[QStringLiteral("x")] = pt.x();
        ptObj[QStringLiteral("y")] = pt.y();
        ptsArray.append(ptObj);
    }
    json[QStringLiteral("controlPoints")] = ptsArray;

    json[QStringLiteral("referencePoint")] = QJsonObject{
        {QStringLiteral("x"), m_referencePoint.x()},
        {QStringLiteral("y"), m_referencePoint.y()}
    };
}

void CustomObject::loadFromJson(const QJsonObject& json)
{
    BaseObject::loadFromJson(json);

    m_refractiveIndex = json[QStringLiteral("refractiveIndex")].toDouble(1.0);

    m_controlPoints.clear();
    const auto ptsArray = json[QStringLiteral("controlPoints")].toArray();
    for (const auto& val : ptsArray) {
        QJsonObject ptObj = val.toObject();
        m_controlPoints.append(QPointF(
            ptObj[QStringLiteral("x")].toDouble(),
            ptObj[QStringLiteral("y")].toDouble()
        ));
    }

    QJsonObject refObj = json[QStringLiteral("referencePoint")].toObject();
    m_referencePoint = QPointF(
        refObj[QStringLiteral("x")].toDouble(0.0),
        refObj[QStringLiteral("y")].toDouble(0.0)
    );
}

QString CustomObject::toRhinoScript(bool exchangeYZ) const
{
    if (m_controlPoints.isEmpty()) return QString();
    return pointsToRhinoFormat(m_controlPoints, exchangeYZ);
}

QString CustomObject::toTxtFormat() const
{
    QStringList lines;
    lines << m_name;
    for (const auto& pt : m_controlPoints) {
        lines << QStringLiteral("%1 %2").arg(pt.x(), 0, 'f', 8).arg(pt.y(), 0, 'f', 8);
    }
    return lines.join(QStringLiteral("\n"));
}

CustomObject* CustomObject::clone(QObject* parent) const
{
    auto* obj = new CustomObject(m_type, m_name, parent);
    obj->m_uuid = m_uuid;
    obj->m_refractiveIndex = m_refractiveIndex;
    obj->m_controlPoints = m_controlPoints;
    obj->m_referencePoint = m_referencePoint;
    obj->m_visible = m_visible;
    obj->m_showLabel = m_showLabel;
    obj->m_showNormals = m_showNormals;
    obj->m_raysQty = m_raysQty;
    obj->m_raysLen = m_raysLen;
    obj->m_normalFlipped = m_normalFlipped;
    obj->m_normalColor = m_normalColor;
    obj->m_selectedColor = m_selectedColor;
    return obj;
}

QString CustomObject::pointsToRhinoFormat(const QVector<QPointF>& pts, bool exchangeYZ) const
{
    QStringList script;
    script << QStringLiteral("! // Object: %1").arg(m_name);
    script << QStringLiteral("_InterpCrv");
    for (const auto& pt : pts) {
        if (exchangeYZ)
            script << QStringLiteral("%1,%2,0").arg(pt.x(), 0, 'f', 8).arg(pt.y(), 0, 'f', 8);
        else
            script << QStringLiteral("%1,%2,0").arg(pt.x(), 0, 'f', 8).arg(pt.y(), 0, 'f', 8);
    }
    script << QStringLiteral("_Enter");
    return script.join(QStringLiteral("\n"));
}

} // namespace ExpressDesigner