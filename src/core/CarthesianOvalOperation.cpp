#include "CarthesianOvalOperation.h"
#include "RefPointDescriptor.h"
#include "Project.h"
#include "CurveObject.h"
#include <geometry/SISLWrapper.h>
#include <QtMath>

namespace ExpressDesigner {

CarthesianOvalOperation::CarthesianOvalOperation(const QString& name, QObject* parent)
    : CustomOperation(OperationType::CartesianOval, name, parent)
{
    m_paramNames << QStringLiteral("WF1") << QStringLiteral("WF2")
                 << QStringLiteral("ReferencePoint") << QStringLiteral("Result");
}

CarthesianOvalOperation::~CarthesianOvalOperation() = default;

static double distSq(const QPointF& a, const QPointF& b)
{
    double dx = a.x() - b.x();
    double dy = a.y() - b.y();
    return dx * dx + dy * dy;
}

// CalcCarthesianPoint2D — continuous version using SISLCurve
// Given WF1 point p1 + normal n1, find X along ray p1 + n1*t such that
// OPL = n1 * dist(p1,X) + n2 * dist(closestPointOnWF2,X) = target C
static bool calcCarthesianPoint2D(
    const QPointF& p1, const QPointF& n1, double n1_idx,
    Geometry::SISLCurve& curve2, double n2_idx,
    double OpticalPathLen, QPointF& result)
{
    const int maxIter = 90;
    const double tolerance = 1e-5;

    auto oplError = [&](double t, QPointF& X, QPointF& p2, QPointF& n2) -> double {
        double dist1 = t * n1_idx;
        X = p1 + n1 * dist1;
        double ct;
        p2 = curve2.closestPoint(X, &ct);
        n2 = curve2.normalAt(p2);

        QPointF v1 = X - p1;
        QPointF v2 = X - p2;
        double d1 = v1.x() * n1.x() + v1.y() * n1.y();
        double d2 = v2.x() * n2.x() + v2.y() * n2.y();
        double OPL = d1 * n1_idx + d2 * n2_idx;
        return OpticalPathLen - OPL;
    };

    double delta_t = OpticalPathLen / 40.0;
    double ta = OpticalPathLen / 2.0;
    double tb = ta + delta_t;

    QPointF Xa, Xb, p2a, p2b, n2a, n2b;

    // --- Secant iteration ---
    double fa = oplError(ta, Xa, p2a, n2a);
    double fb = oplError(tb, Xb, p2b, n2b);
    int count = 0;

    while (qAbs(fb) > tolerance && count < maxIter) {
        if (qAbs(fa - fb) > 1e-12) {
            double tc = ta - fa * (tb - ta) / (fb - fa);
            ta = tb; fa = fb;
            tb = tc;
        } else {
            tb += delta_t;
        }
        fb = oplError(tb, Xb, p2b, n2b);
        count++;
    }

    // --- Fallback 1: adaptive stepping forward ---
    if (qAbs(fb) > tolerance) {
        double factor = 1.0, sent = 1, exini = 0;
        int flag = 0;
        tb = OpticalPathLen / 40.0;
        count = 0;
        do {
            fb = oplError(tb, Xb, p2b, n2b);
            if (flag == 0) exini = fb;
            if (flag && fb * sent * factor < 0) {
                factor *= 0.5; sent = -sent;
            } else if (flag) {
                factor = qMin(factor * 1.2, 5.0);
            }
            tb += factor * sent * delta_t;
            flag = 1;
            count++;
        } while (qAbs(fb) > tolerance && count < maxIter);
    }

    // --- Fallback 2: adaptive stepping reverse ---
    if (qAbs(fb) > tolerance) {
        double factor = 1.0, sent = -1;
        tb = OpticalPathLen / 40.0;
        count = 0;
        do {
            fb = oplError(tb, Xb, p2b, n2b);
            if (fb * sent * factor < 0) {
                factor *= 0.5; sent = -sent;
            } else {
                factor = qMin(factor * 1.2, 5.0);
            }
            tb += factor * sent * delta_t;
            count++;
        } while (qAbs(fb) > tolerance && count < maxIter);
    }

    result = Xb;
    return qAbs(fb) <= tolerance;
}

bool CarthesianOvalOperation::execute(Project* project)
{
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
    // Use ref point kind + sourceName to resolve coordinates
    if (!m_refPointSourceName.isEmpty() && project) {
        CustomObject* refObj = project->findObject(m_refPointSourceName);
        if (refObj && refObj->controlPointCount() > 0) {
            const auto& pts = refObj->controlPoints();
            if (m_refPointKind == static_cast<int>(RefPointDescriptor::CurveEnd) && pts.size() > 1)
                refPoint = pts.last();
            else
                refPoint = pts.first(); // PointObject or CurveBegin
        }
    } else {
        // Legacy fallback
        CustomObject* refObj = project->findObject(m_paramNames.value(PARAM_REF_POINT));
        if (refObj && !refObj->controlPoints().isEmpty())
            refPoint = refObj->controlPoints().first();
    }

    double n1 = wf1->refractiveIndex();
    double n2 = wf2->refractiveIndex();
    bool flip1 = wf1->isNormalFlipped();
    int nPts = m_amountOfPoints;
    if (nPts < 2) nPts = 100;

    // Continuous curves — no discretization
    Geometry::SISLCurve curve1(wf1->controlPoints(), 3, true);
    Geometry::SISLCurve curve2(wf2->controlPoints(), 3, true);

    // Compute OPL constant C at reference point
    // Use Find_CP_Repaired: the closest point whose normal passes through refPoint
    auto repairedPoint = [](Geometry::SISLCurve& curve, const QPointF& ref) -> QPointF {
        double t;
        QPointF cp = curve.closestPoint(ref, &t);
        QPointF n = curve.normalAt(cp);              // normal at seed
        double proj = n.x() * (cp.x() - ref.x()) + n.y() * (cp.y() - ref.y());
        return QPointF(ref.x() + n.x() * proj, ref.y() + n.y() * proj);
    };
    QPointF rp1 = repairedPoint(curve1, refPoint);
    QPointF rp2 = repairedPoint(curve2, refPoint);
    double d1ref = qSqrt(distSq(rp1, refPoint));
    double d2ref = qSqrt(distSq(rp2, refPoint));
    double C = n1 * d1ref + n2 * d2ref;

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
        calcCarthesianPoint2D(p1, n1v, n1, curve2, n2, C, ovalPt);
        ovalPts.append(ovalPt);
    }

    // Force reference point onto curve (continuous closest)
    int closestIdx = 0;
    double closestDist = 1e18;
    for (int i = 0; i < ovalPts.size(); ++i) {
        double d = distSq(ovalPts[i], refPoint);
        if (d < closestDist) { closestDist = d; closestIdx = i; }
    }
    ovalPts[closestIdx] = refPoint;

    if (ovalPts.size() < 3) {
        delete result;
        m_errorCode = 1;
        m_errorMessage = tr("Cartesian oval produced less than 3 points — operation failed.");
        emit operationExecuted(false);
        return false;
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
}

void CarthesianOvalOperation::loadFromJson(const QJsonObject& json)
{
    CustomOperation::loadFromJson(json);
}

} // namespace ExpressDesigner