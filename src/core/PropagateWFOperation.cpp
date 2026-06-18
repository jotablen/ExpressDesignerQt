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

// Ray-segment intersection: solve origin + t*dir = a + u*(b-a), t>=0, 0<=u<=1
static double raySegmentHit(const QPointF& origin, const QPointF& dir,
                             const QPointF& a, const QPointF& b)
{
    QPointF seg = b - a;
    double denom = dir.x() * seg.y() - dir.y() * seg.x();
    if (qAbs(denom) < 1e-12) return -1.0;
    QPointF diff = a - origin;
    double t = (diff.x() * seg.y() - diff.y() * seg.x()) / denom;
    double u = (diff.x() * dir.y() - diff.y() * dir.x()) / -denom;
    if (t >= 0.0 && u >= 0.0 && u <= 1.0)
        return t;
    return -1.0;
}

// Find closest ray-curve intersection. Returns intersection point, sets hitSeg and hitDist.
static QPointF rayCurveIntersection(const QPointF& origin, const QPointF& dir,
                                     const QVector<QPointF>& curve, int* hitSeg,
                                     double* hitDist)
{
    double bestT = 1e18;
    int bestSeg = -1;
    int m = curve.size();
    for (int i = 0; i < m - 1; ++i) {
        double t = raySegmentHit(origin, dir, curve[i], curve[i + 1]);
        if (t >= 0.0 && t < bestT) {
            bestT = t;
            bestSeg = i;
        }
    }
    if (bestSeg >= 0) {
        *hitDist = bestT;
        *hitSeg = bestSeg;
        return origin + dir * bestT;
    }
    return QPointF(); // (0,0) if no hit
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
    if (n2 <= 0.0) n2 = n1;
    double fDistance = m_offset;
    if (fDistance <= 0.0) fDistance = 1.0;
    bool flipWF = wf->isNormalFlipped();
    int numPoints = m_amountOfPoints;
    if (numPoints < 2) numPoints = 100;

    // Build SISL curves — use continuous normals (no discretization)
    Geometry::SISLCurve wfCurve(wf->controlPoints(), 3, true);
    Geometry::SISLCurve surfCurve(surface->controlPoints(), 3, true);

    QVector<QPointF> wfPts = wfCurve.evaluateAll(numPoints);

    auto* result = new CurveObject(resultName());
    result->setObjectType(withWavefront(withResult(ObjectType::Curve)));
    result->setRefractiveIndex(n2);
    result->setNormalParams(wf->normalRaysQty(), wf->normalRaysLen());
    if (wf->isNormalFlipped())
        result->flipNormal();

    QVector<QPointF> propagatedPts;
    propagatedPts.reserve(numPoints);

    for (int i = 0; i < numPoints; ++i) {
        double t = numPoints > 1 ? static_cast<double>(i) / (numPoints - 1) : 0.0;
        QPointF srcPt = wfPts[i];
        QPointF norm = wfCurve.normal(t, flipWF);

        // Step 1: Cast ray from WF point along normal to intersect the surface (continuous)
        double hitDist = surfCurve.rayIntersection(srcPt, norm);

        if (hitDist < 0.0) {
            continue; // No hit — discard
        }

        QPointF surfHit = srcPt + norm * hitDist;

        // Step 2: Optical path length from WF to surface
        double OPL = n1 * hitDist;
        double remaining = fDistance - OPL;

        // Surface normal at hit point — use continuous curve normal
        QPointF surfNormal = surfCurve.normalAt(surfHit);

        // Snell's law deflection — vectorial approach with surface normal
        bool tir = false;
        QPointF deflectedDir = Optics::getDeflectedVector(norm, surfNormal, n1, n2, tir);

        if (!tir)
            remaining /= n2; // transmitted

        QPointF resultPt = surfHit + deflectedDir * remaining;
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