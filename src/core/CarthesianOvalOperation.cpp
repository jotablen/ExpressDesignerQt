#include "CarthesianOvalOperation.h"
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

// Helper: squared distance
static double distSq(const QPointF& a, const QPointF& b)
{
    double dx = a.x() - b.x();
    double dy = a.y() - b.y();
    return dx * dx + dy * dy;
}

// Given p1 (fixed), solve OPL(p1, X, p2) = C for t on curve2.
// Returns 0 if root found, -1 if no solution within parameter range.
// The oval point X is stored in *outPt.
static int solveOvalPointOnCurve(
    Geometry::SISLCurve& curve1, Geometry::SISLCurve& curve2,
    double t1, double n1, double n2, double C,
    QPointF& outPt)
{
    QPointF p1 = curve1.evaluate(t1);

    // OPL function: for a given t2 parameter on curve2,
    // F(t2) = OPL_line(p1, curve2(t2)) - C
    auto F = [&](double t2) -> double {
        QPointF p2 = curve2.evaluate(t2);
        double L = qSqrt(distSq(p1, p2));
        if (L < 1e-12) return -C;  // coincident → OPL = 0

        // Solve for t_L on the line X = p1 + t_L * (p2-p1):
        //   n1 * t_L * L + n2 * (1-t_L) * L = C
        //   t_L = (C/L - n2) / (n1 - n2)
        double C_L = C / L;
        double denom = n1 - n2;
        double t_L;
        if (qAbs(denom) < 1e-12) {
            t_L = 0.5;  // equal indices → midpoint
        } else {
            t_L = (C_L - n2) / denom;
        }
        double tL_clamped = qBound(0.0, t_L, 1.0);
        double OPL = n1 * tL_clamped * L + n2 * (1.0 - tL_clamped) * L;
        return OPL - C;
    };

    // Secant method
    constexpr int kMaxIter = 50;
    constexpr double kTol = 1e-8;
    constexpr double kEps = 1e-7;

    // Initial guesses: scan the t2 range for sign changes
    double tA = 0.0;
    double tB = 1.0;
    double fA = F(tA);
    double fB = F(tB);

    // If root at endpoints
    if (qAbs(fA) < kTol) {
        QPointF p2 = curve2.evaluate(tA);
        double L = qSqrt(distSq(p1, p2));
        if (L < 1e-12) { outPt = p1; return 0; }
        double C_L = C / L;
        double t_L = (qAbs(n1-n2) < 1e-12) ? 0.5 : (C_L - n2) / (n1 - n2);
        t_L = qBound(0.0, t_L, 1.0);
        outPt = p1 + (p2 - p1) * t_L;
        return 0;
    }
    if (qAbs(fB) < kTol) {
        QPointF p2 = curve2.evaluate(tB);
        double L = qSqrt(distSq(p1, p2));
        if (L < 1e-12) { outPt = p1; return 0; }
        double C_L = C / L;
        double t_L = (qAbs(n1-n2) < 1e-12) ? 0.5 : (C_L - n2) / (n1 - n2);
        t_L = qBound(0.0, t_L, 1.0);
        outPt = p1 + (p2 - p1) * t_L;
        return 0;
    }

    // If no sign change in [0,1], just use the point that minimizes |F|
    if (fA * fB > 0) {
        double bestT = (qAbs(fA) < qAbs(fB)) ? tA : tB;
        QPointF p2 = curve2.evaluate(bestT);
        double L = qSqrt(distSq(p1, p2));
        if (L < 1e-12) { outPt = p1; return 0; }
        double C_L = C / L;
        double t_L = (qAbs(n1-n2) < 1e-12) ? 0.5 : (C_L - n2) / (n1 - n2);
        t_L = qBound(0.0, t_L, 1.0);
        outPt = p1 + (p2 - p1) * t_L;
        return 0;
    }

    // Sign change exists → secant
    double tPrev = (qAbs(fA) < qAbs(fB)) ? tA : tB;
    double fPrev = F(tPrev);

    for (int iter = 0; iter < kMaxIter; ++iter) {
        // Secant step: find where secant line crosses zero
        double fA_cur = F(tA);
        double fB_cur = F(tB);
        if (qAbs(fA_cur) < kTol || qAbs(fB_cur) < kTol) break;

        double tMid = (tA * fB_cur - tB * fA_cur) / (fB_cur - fA_cur);
        tMid = qBound(tA + kEps, tMid, tB - kEps);

        double fMid = F(tMid);
        if (qAbs(fMid) < kTol) {
            tA = tB = tMid;
            break;
        }

        // Update bracket
        if (fA_cur * fMid < 0) {
            tB = tMid;
        } else {
            tA = tMid;
        }
    }

    // Use midpoint of bracket
    double tMid = (tA + tB) * 0.5;
    QPointF p2 = curve2.evaluate(tMid);
    double L = qSqrt(distSq(p1, p2));
    if (L < 1e-12) { outPt = p1; return 0; }
    double C_L = C / L;
    double t_L = (qAbs(n1-n2) < 1e-12) ? 0.5 : (C_L - n2) / (n1 - n2);
    t_L = qBound(0.0, t_L, 1.0);
    outPt = p1 + (p2 - p1) * t_L;
    return 0;
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

    CustomObject* refObj = project->findObject(m_paramNames.value(PARAM_REF_POINT));
    QPointF refPoint(0.0, 0.0);
    if (refObj && !refObj->controlPoints().isEmpty())
        refPoint = refObj->controlPoints().first();

    double n1 = wf1->refractiveIndex();
    double n2 = wf2->refractiveIndex();
    int n = m_amountOfPoints;
    if (n < 2) n = 100;

    // Build SISL curves — operate against curves, not discretized point lists
    Geometry::SISLCurve curve1(wf1->controlPoints(), 3, true);
    Geometry::SISLCurve curve2(wf2->controlPoints(), 3, true);

    // --- Step 1: Find the optical path length constant C ---
    // Evaluate both curves finely to find the closest point to the reference
    QVector<QPointF> pts1 = curve1.evaluateAll(4 * n);
    QVector<QPointF> pts2 = curve2.evaluateAll(4 * n);

    int ci1 = 0, ci2 = 0;
    double bestD1 = 1e18, bestD2 = 1e18;
    for (int i = 0; i < pts1.size(); ++i) {
        double d = distSq(pts1[i], refPoint);
        if (d < bestD1) { bestD1 = d; ci1 = i; }
    }
    for (int i = 0; i < pts2.size(); ++i) {
        double d = distSq(pts2[i], refPoint);
        if (d < bestD2) { bestD2 = d; ci2 = i; }
    }

    double d1ref = qSqrt(bestD1);
    double d2ref = qSqrt(bestD2);
    double C = n1 * d1ref + n2 * d2ref;
    if (C < 1e-12) {
        // Try with a nearby point
        for (int i = 0; i < pts1.size() && C < 1e-12; ++i) {
            double d1 = qSqrt(distSq(pts1[i], refPoint));
            double d2 = qSqrt(distSq(pts2[qMin(i, pts2.size()-1)], refPoint));
            C = n1 * d1 + n2 * d2;
        }
    }

    auto* result = new CurveObject(resultName());
    result->setObjectType(withResult(ObjectType::Curve));
    result->setRefractiveIndex((n1 + n2) * 0.5);

    // --- Step 2: For each sample on curve1, find oval point via secant on curve2 ---
    QVector<QPointF> ovalPts;
    ovalPts.reserve(n);

    for (int i = 0; i < n; ++i) {
        double t1 = static_cast<double>(i) / (n - 1);
        QPointF ovalPt;
        if (solveOvalPointOnCurve(curve1, curve2, t1, n1, n2, C, ovalPt) >= 0) {
            ovalPts.append(ovalPt);
        }
    }

    // If we got fewer points than expected, pad with the last point
    while (ovalPts.size() < static_cast<qsizetype>(n))
        ovalPts.append(ovalPts.isEmpty() ? QPointF() : ovalPts.last());

    // --- Step 3: Ensure reference point is part of the curve ---
    int closestIdx = 0;
    double closestDist = 1e18;
    for (int i = 0; i < ovalPts.size(); ++i) {
        double d = distSq(ovalPts[i], refPoint);
        if (d < closestDist) { closestDist = d; closestIdx = i; }
    }
    ovalPts[closestIdx] = refPoint;

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