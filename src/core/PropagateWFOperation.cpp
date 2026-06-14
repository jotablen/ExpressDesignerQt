#include "PropagateWFOperation.h"
#include "Project.h"
#include "CurveObject.h"
#include <geometry/SISLWrapper.h>
#include <optics/SnellLaw.h>
#include <QtMath>

namespace ExpressDesigner {

PropagateWFOperation::PropagateWFOperation(const QString& name, QObject* parent)
    : CustomOperation(OperationType::PropagateWF, name, parent)
{
    m_paramNames << QStringLiteral("WF") << QStringLiteral("Surface")
                 << QStringLiteral("IOR") << QStringLiteral("Offset")
                 << QStringLiteral("Result");
}

PropagateWFOperation::~PropagateWFOperation() = default;

// Compute outward normal at a sample point on a curve
static QPointF normalAt(const QVector<QPointF>& pts, int idx, bool flipped)
{
    if (pts.size() < 2) return QPointF(1.0, 0.0);
    int n = pts.size();
    int prev = (idx - 1 + n) % n;
    int next = (idx + 1) % n;
    QPointF dir = pts[next] - pts[prev];
    double len = qSqrt(dir.x() * dir.x() + dir.y() * dir.y());
    if (len < 1e-12) return QPointF(1.0, 0.0);
    QPointF norm(-dir.y() / len, dir.x() / len);
    if (flipped) norm = -norm;
    return norm;
}

// Ray-line intersection: given origin + direction, find intersection with segment a→b
// Returns parameter t such that origin + t * dir hits the segment. Returns -1 if no hit.
static double raySegmentHit(const QPointF& origin, const QPointF& dir,
                             const QPointF& a, const QPointF& b)
{
    QPointF seg = b - a;
    // Solve: origin + t*dir = a + u*seg,   t >= 0, 0 <= u <= 1
    double denom = dir.x() * seg.y() - dir.y() * seg.x();
    if (qAbs(denom) < 1e-12) return -1.0;
    QPointF diff = a - origin;
    double t = (diff.x() * seg.y() - diff.y() * seg.x()) / denom;
    double u = (diff.x() * dir.y() - diff.y() * dir.x()) / -denom;
    if (t >= 0.0 && u >= 0.0 && u <= 1.0)
        return t;
    return -1.0;
}

// Find the closest intersection of a ray (origin + dir) with a curve defined by a point list.
// Returns the intersection point and the segment index. Sets *hitDist to the distance.
static QPointF rayCurveIntersection(const QPointF& origin, const QPointF& dir,
                                     const QVector<QPointF>& curve, int* hitSeg,
                                     double* hitDist)
{
    QPointF bestPt;
    double bestT = 1e18;
    int bestSeg = -1;
    int m = curve.size();

    // Try all segments (open curve — last point connects to first for closed? Assume open)
    for (int i = 0; i < m - 1; ++i) {
        double t = raySegmentHit(origin, dir, curve[i], curve[i + 1]);
        if (t >= 0.0 && t < bestT) {
            bestT = t;
            bestSeg = i;
        }
    }
    if (bestSeg >= 0) {
        bestPt = origin + dir * bestT;
        *hitDist = bestT;
        *hitSeg = bestSeg;
    }
    return bestPt; // (0,0) if no hit
}

bool PropagateWFOperation::execute(Project* project)
{
    m_errorCode = 0;
    if (!project) {
        m_errorCode = -1;
        return false;
    }

    CustomObject* wf = project->findObject(m_paramNames.value(PARAM_WF));
    CustomObject* surface = project->findObject(m_paramNames.value(PARAM_SURFACE));
    if (!wf || !surface) {
        m_errorCode = -2;
        return false;
    }

    double n1 = wf->refractiveIndex();
    double n2 = m_paramNames.value(PARAM_IOR).toDouble();
    if (n2 <= 0.0) n2 = n1; // default to same index
    double offset = m_offset;
    bool flipWF = wf->isNormalFlipped();
    int numPoints = m_amountOfPoints;
    if (numPoints < 2) numPoints = 100;

    // Build SISL curves from both objects and sample uniformly
    Geometry::SISLCurve wfCurve(wf->controlPoints(), 3, true);
    Geometry::SISLCurve surfCurve(surface->controlPoints(), 3, true);

    QVector<QPointF> wfPts = wfCurve.evaluateAll(numPoints);
    QVector<QPointF> surfPts = surfCurve.evaluateAll(numPoints);

    auto* result = new CurveObject(resultName());
    result->setObjectType(withWavefront(withResult(ObjectType::Curve)));
    result->setRefractiveIndex(n2);
    result->setNormalParams(wf->normalRaysQty(), wf->normalRaysLen());
    if (wf->isNormalFlipped())
        result->flipNormal();

    QVector<QPointF> propagatedPts;
    propagatedPts.reserve(numPoints);

    for (int i = 0; i < numPoints; ++i) {
        QPointF srcPt = wfPts[i];
        QPointF norm = normalAt(wfPts, i, flipWF);

        // Step 1: Cast ray from source WF point along its normal to find intersection with surface
        int hitSeg = -1;
        double hitDist = 0.0;
        QPointF surfHit = rayCurveIntersection(srcPt, norm, surfPts, &hitSeg, &hitDist);

        if (hitSeg < 0) {
            // No hit — keep the original point propagated by the offset along normal
            propagatedPts.append(srcPt + norm * offset);
            continue;
        }

        // Step 2: Surface normal at intersection point (from surface curve)
        // Interpolate between segment endpoints
        QPointF segA = surfPts[hitSeg];
        QPointF segB = surfPts[qMin(hitSeg + 1, surfPts.size() - 1)];
        QPointF segDir = segB - segA;
        double segLen = qSqrt(segDir.x() * segDir.x() + segDir.y() * segDir.y());
        QPointF surfNormal;
        if (segLen > 1e-9) {
            surfNormal = QPointF(-segDir.y() / segLen, segDir.x() / segLen);
        } else {
            surfNormal = QPointF(0.0, 1.0);
        }

        // Step 3: Snell's law deflection at the surface point
        // Normal direction for Snell: should point toward the incident ray
        double dotIn = norm.x() * surfNormal.x() + norm.y() * surfNormal.y();
        QPointF snellNormal = (dotIn < 0) ? surfNormal : QPointF(-surfNormal.x(), -surfNormal.y());

        bool tir = false;
        QPointF deflectedDir = Optics::getDeflectedVector(norm, surfHit, n1, n2, tir);

        // Step 4: Place result point at the deflected direction by the offset distance
        QPointF resultPt;
        if (tir) {
            // Total internal reflection — use reflected direction (as computed by getDeflectedVector)
            resultPt = surfHit + deflectedDir * offset;
        } else {
            resultPt = surfHit + deflectedDir * offset;
        }

        propagatedPts.append(resultPt);
    }

    result->setControlPoints(propagatedPts);
    project->addResultObject(result);

    emit operationExecuted(true);
    return true;
}

bool PropagateWFOperation::isParamObject(int index) const
{
    return index == PARAM_WF || index == PARAM_SURFACE;
}

QString PropagateWFOperation::resultName() const
{
    return m_name;
}

QString PropagateWFOperation::paramPrefixOnTree(int index) const
{
    switch (index) {
    case PARAM_WF: return QStringLiteral("WF: ");
    case PARAM_SURFACE: return QStringLiteral("Srf: ");
    case PARAM_IOR: return QStringLiteral("IOR: ");
    case PARAM_OFFSET: return QStringLiteral("Off: ");
    case PARAM_RESULT: return QStringLiteral("Res: ");
    default: return CustomOperation::paramPrefixOnTree(index);
    }
}

double PropagateWFOperation::offset() const { return m_offset; }
void PropagateWFOperation::setOffset(double value) { m_offset = value; }

void PropagateWFOperation::saveToJson(QJsonObject& json) const
{
    CustomOperation::saveToJson(json);
    json[QStringLiteral("operationSubType")] = QStringLiteral("propagateWF");
    json[QStringLiteral("offset")] = m_offset;
}

void PropagateWFOperation::loadFromJson(const QJsonObject& json)
{
    CustomOperation::loadFromJson(json);
    m_offset = json[QStringLiteral("offset")].toDouble(0.0);
}

} // namespace ExpressDesigner