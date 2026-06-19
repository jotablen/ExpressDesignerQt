#include "PropagateWFOperation.h"
#include "Project.h"
#include "CurveObject.h"
#include <geometry/SISLWrapper.h>
#include <optics/SnellLaw.h>
#include <QtMath>
#include <utils/Logger.h>

namespace ExpressDesigner {

PropagateWFOperation::PropagateWFOperation(const QString& name, QObject* parent)
    : CustomOperation(OperationType::PropagateWF, name, parent)
{
    m_paramNames << QStringLiteral("WF") << QStringLiteral("Surface")
                 << QStringLiteral("IOR") << QStringLiteral("Offset")
                 << QStringLiteral("Result");
}

PropagateWFOperation::~PropagateWFOperation() = default;

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

    // Continuous curves (no discretization)
    Geometry::SISLCurve wfCurve(wf->controlPoints(), 3, true);
    Geometry::SISLCurve surfCurve(surface->controlPoints(), 3, true);

    LOG_INFO(QStringLiteral("GEO"), QStringLiteral("PropgWF: wf=%1 pts, surf=%2 pts, offset=%.3f")
             .arg(wf->controlPoints().size()).arg(surface->controlPoints().size()).arg(fDistance));

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

        // Step 1: Curve-plane intersection (ODs SISLDrink algorithm)
        // The plane passes through srcPt and its normal is perpendicular
        // to the ray direction (cross with Z axis): planeNormal = (ny, -nx).
        // This makes the plane CONTAIN the ray, and intersecting the surface
        // curve against this plane gives the exact hit point.
        QPointF planeNormal(-norm.y(), norm.x());
        QPointF surfHit;
        surfCurve.planeIntersection(srcPt, planeNormal, &surfHit);

        if (surfHit.isNull()) {
            continue; // No intersection — discard
        }

        // Step 2: Optical path length from WF to surface
        // Use Euclidean distance from srcPt to surfHit (plane normal is perpendicular
        // to ray direction, so planeIntersection distance is not along the ray)
        double dx = surfHit.x() - srcPt.x();
        double dy = surfHit.y() - srcPt.y();
        double hitDist = qSqrt(dx * dx + dy * dy);
        double OPL = n1 * hitDist;
        double remaining = fDistance - OPL;

        // Surface normal at hit point — continuous curve normal
        QPointF surfNormal = surfCurve.normalAt(surfHit);

        // Snell's law deflection — vectorial approach with surface normal
        bool tir = false;
        QPointF deflectedDir = Optics::getDeflectedVector(norm, surfNormal, n1, n2, tir);

        if (!tir)
            remaining /= n2; // transmitted

        QPointF resultPt = surfHit + deflectedDir * remaining;
        propagatedPts.append(resultPt);
    }

    if (propagatedPts.size() < 3) {
        delete result;
        m_errorCode = 1;
        m_errorMessage = tr("Propagation produced less than 3 points — operation failed.");
        emit operationExecuted(false);
        return false;
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