#include "CarthesianOvalOperation.h"
#include "RefPointDescriptor.h"
#include "Project.h"
#include "CurveObject.h"
#include <geometry/SISLWrapper.h>
#include <geometry/VectorUtils.h>
#include <QtMath>

namespace ExpressDesigner {

CarthesianOvalOperation::CarthesianOvalOperation(const QString& name, QObject* parent)
    : CustomOperation(OperationType::CartesianOval, name, parent)
{
    m_paramNames << QStringLiteral("WF1") << QStringLiteral("WF2")
                 << QStringLiteral("ReferencePoint") << QStringLiteral("Result");
}

CarthesianOvalOperation::~CarthesianOvalOperation() = default;

/// CalcCarthesianPoint2D — continuous version using SISLCurve
/// Given WF1 point p1 + normal n1, find X along ray p1 + n1*t such that
/// OPL = n1 * dist(p1,X) + n2 * dist(closestPointOnWF2,X) = target C
static bool calcCarthesianPoint2D(
    const QPointF& p1, const QPointF& n1, double n1_idx,
    Geometry::SISLCurve& curve2, double n2_idx, bool flip2,
    double OpticalPathLen, QPointF& result)
{
    using namespace Geometry;

    // ODs-derived constants: match Funciones_SMS.cpp CalculaPtoAnal
    constexpr int    maxIter    = 90;      // secant + fallback iterations
    constexpr double convTol    = 1e-6;    // OPL convergence tolerance (ODs: 1e-6)
    constexpr double denomEps   = 1e-12;   // secant denominator floor
    constexpr double sentinel   = 1e9;     // oplError sentinel when normal is missing

    /// ODs CalculaPtoAnal: t is PHYSICAL distance along n1 from p1
    /// OPL(X) = n1 * (X-p1)·n1 + n2 * (X-cp)·n2   (signed, same convention as C)
    /// f(t) = OPL(X) - target → iterate to 0
    auto oplError = [&](double t, QPointF& XOut, QPointF& cpOut, QPointF& n2Out) -> double {
        XOut = p1 + n1 * t;
        if (!curve2.closestPointN(XOut, cpOut, n2Out, flip2))
            return sentinel; // normal doesn't pass through XOut → skip
        return n1_idx * dot(p1 - XOut, n1) + n2_idx * dot(cpOut - XOut, n2Out) - OpticalPathLen;
    };

    const double delta_t = OpticalPathLen / 40.0;
    double ta = OpticalPathLen / 2.0;
    double tb = ta + delta_t;
    QPointF Xa, Xb, p2a, p2b, n2a, n2b;

    // ── Secant iteration ──
    double fa = oplError(ta, Xa, p2a, n2a);
    double fb = oplError(tb, Xb, p2b, n2b);
    int count = 0;
    while (qAbs(fb) > convTol && count < maxIter) {
        if (qAbs(fa - fb) > denomEps) {
            double tc = ta - fa * (tb - ta) / (fb - fa);
            ta = tb; fa = fb; tb = tc;
        } else {
            tb += delta_t;
        }
        fb = oplError(tb, Xb, p2b, n2b);
        ++count;
    }

    // ── Fallback 1: adaptive stepping forward ──
    if (qAbs(fb) > convTol) {
        double factor = 1.0, sent = 1, exini = 0;
        int flag = 0;
        tb = OpticalPathLen / 40.0;
        count = 0;
        do {
            fb = oplError(tb, Xb, p2b, n2b);
            if (flag == 0) exini = fb;
            if (flag && fb * sent * factor < 0) { factor *= 0.5; sent = -sent; }
            else if (flag) { factor = qMin(factor * 1.2, 5.0); }
            tb += factor * sent * delta_t;
            flag = 1;
            ++count;
        } while (qAbs(fb) > convTol && count < maxIter);
    }

    // ── Fallback 2: adaptive stepping reverse ──
    if (qAbs(fb) > convTol) {
        double factor = 1.0, sent = -1;
        tb = OpticalPathLen / 40.0;
        count = 0;
        do {
            fb = oplError(tb, Xb, p2b, n2b);
            if (fb * sent * factor < 0) { factor *= 0.5; sent = -sent; }
            else { factor = qMin(factor * 1.2, 5.0); }
            tb += factor * sent * delta_t;
            ++count;
        } while (qAbs(fb) > convTol && count < maxIter);
    }

    result = Xb;
    return qAbs(fb) <= convTol;
}

// ============================================================================
bool CarthesianOvalOperation::execute(Project* project)
{
    using namespace Geometry;

    m_errorCode = 0;
    if (!project) {
        m_errorCode = -1;
        m_errorMessage = QStringLiteral("No project provided");
        return false;
    }

    CustomObject* wf1 = project->findObject(m_paramNames.value(PARAM_WF1));
    CustomObject* wf2 = project->findObject(m_paramNames.value(PARAM_WF2));
    if (!wf1 || !wf2) {
        m_errorCode = -2;
        m_errorMessage = QStringLiteral("Wavefront objects not found");
        return false;
    }

    QPointF refPoint(0.0, 0.0);
    if (!m_refPointSourceName.isEmpty() && project) {
        CustomObject* refObj = project->findObject(m_refPointSourceName);
        if (refObj && refObj->controlPointCount() > 0) {
            const auto& pts = refObj->controlPoints();
            if (m_refPointKind == static_cast<int>(RefPointDescriptor::CurveEnd) && pts.size() > 1)
                refPoint = pts.last();
            else
                refPoint = pts.first();
        }
    } else {
        CustomObject* refObj = project->findObject(m_paramNames.value(PARAM_REF_POINT));
        if (refObj && !refObj->controlPoints().isEmpty())
            refPoint = refObj->controlPoints().first();
    }

    double n1 = wf1->refractiveIndex();
    double n2 = wf2->refractiveIndex();
    bool flip1 = wf1->isNormalFlipped();
    bool flip2 = wf2->isNormalFlipped();
    int nPts = m_amountOfPoints;
    if (nPts < 2) nPts = 100;

    Geometry::SISLCurve curve1(wf1->controlPoints(), 3, true);
    Geometry::SISLCurve curve2(wf2->controlPoints(), 3, true);

    // ── Compute OPL constant C at reference point ──
    QPointF n1v, n2v, rp1, rp2;
    curve1.closestPointN(refPoint, rp1, n1v, flip1);
    curve2.closestPointN(refPoint, rp2, n2v, flip2);

    double C = n1 * dot(rp1 - refPoint, n1v)
             + n2 * dot(rp2 - refPoint, n2v);

    auto* result = new CurveObject(resultName());
    result->setObjectType(withResult(ObjectType::Curve));
    result->setRefractiveIndex((n1 + n2) * 0.5);

    QVector<QPointF> ovalPts;
    ovalPts.reserve(nPts);

    for (int i = 0; i < nPts; ++i) {
        double t1 = (nPts > 1) ? static_cast<double>(i) / (nPts - 1) : 0.0;
        QPointF p1 = curve1.evaluate(t1);
        QPointF n1v = curve1.normal(t1, flip1);
        QPointF ovalPt;
        if (calcCarthesianPoint2D(p1, n1v, n1, curve2, n2, flip2, C, ovalPt)) {
            // Post-convergence check: does WF2 normal pass through ovalPt?
            double checkT;
            QPointF cp = curve2.closestPoint(ovalPt, &checkT);
            QPointF n  = curve2.normal(checkT, flip2);
            if (qAbs(cross(n, cp - ovalPt)) <= 1e-10)
                ovalPts.append(ovalPt);
            // else: skip — normal validation failed
        }
        // else: skip — OPL didn't converge
    }

    if (ovalPts.size() < 3) {
        delete result;
        m_errorCode = 1;
        m_errorMessage = tr("Cartesian oval produced less than 3 points — operation failed.");
        emit operationExecuted(false);
        return false;
    }

    // Optional: force reference point onto curve
    if (m_forceRefPoint) {
        int closestIdx = 0;
        double closestDist = 1e18;
        for (int i = 0; i < ovalPts.size(); ++i) {
            double d = distSq(ovalPts[i], refPoint);
            if (d < closestDist) { closestDist = d; closestIdx = i; }
        }
        ovalPts[closestIdx] = refPoint;
    }

    result->setControlPoints(ovalPts);
    project->addResultObject(result);

    emit operationExecuted(true);
    return true;
}

bool CarthesianOvalOperation::isParamObject(int index) const
{
    return index == PARAM_WF1 || index == PARAM_WF2 || index == PARAM_REF_POINT;
}

QString CarthesianOvalOperation::resultName() const { return m_name; }

QString CarthesianOvalOperation::paramPrefixOnTree(int index) const
{
    switch (index) {
    case PARAM_WF1: return QStringLiteral("WF1: ");
    case PARAM_WF2: return QStringLiteral("WF2: ");
    case PARAM_REF_POINT: return QStringLiteral("Ref: ");
    case PARAM_RESULT: return QStringLiteral("Result: ");
    default: return CustomOperation::paramPrefixOnTree(index);
    }
}

void CarthesianOvalOperation::saveToJson(QJsonObject& json) const
{
    CustomOperation::saveToJson(json);
    json[QStringLiteral("operationSubType")] = QStringLiteral("cartesianOval");
    json[QStringLiteral("refPointKind")] = m_refPointKind;
    json[QStringLiteral("refPointSourceName")] = m_refPointSourceName;
    json[QStringLiteral("forceRefPoint")] = m_forceRefPoint;
}

void CarthesianOvalOperation::loadFromJson(const QJsonObject& json)
{
    CustomOperation::loadFromJson(json);
    m_refPointKind = json[QStringLiteral("refPointKind")].toInt(0);
    m_refPointSourceName = json[QStringLiteral("refPointSourceName")].toString();
    m_forceRefPoint = json[QStringLiteral("forceRefPoint")].toBool(true);
}

} // namespace ExpressDesigner