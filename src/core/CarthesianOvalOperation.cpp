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

// Computes a unit outward normal at a sampled curve point.
// Respects the 'flipped' parameter.
static QPointF curveNormal(const QVector<QPointF>& pts, int idx, bool flipped)
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

// Check if the point pto_WF on a wavefront has its normal aligned with the
// direction toward the reference point.
// Returns true if |dot(unit_to_Ref, normal)| ≈ 1 (within ~5e-5 tolerance).
// This is Ovals Designer's IsGoodClosestPoint / GetClosestPointGeneral logic.
static bool isGoodClosestPoint(const QPointF& wfPt, const QPointF& refPt,
                                const QVector<QPointF>& pts, int idx, bool flipped)
{
    QPointF normal = curveNormal(pts, idx, flipped);
    QPointF toRef = refPt - wfPt;
    double toRefLen = qSqrt(toRef.x() * toRef.x() + toRef.y() * toRef.y());
    if (toRefLen < 1e-12) return true; // coincident

    double dot = (normal.x() * toRef.x() + normal.y() * toRef.y()) / toRefLen;
    // Also check flipped normal (the "other side")
    double dotFlipped = -dot;
    double best = qMax(qAbs(dot), qAbs(dotFlipped));
    return qAbs(best - 1.0) < 5.0e-5;
}

// For a given WF, find the sampled point index whose normal best points toward
// the reference point. Returns -1 if nothing good found.
static int findBestNormalIndex(const QVector<QPointF>& pts, const QPointF& refPt,
                                bool flipped, QPointF* outNormal = nullptr)
{
    if (pts.size() < 2) return 0;
    double bestVal = -1.0;
    int bestIdx = 0;
    for (int i = 0; i < pts.size(); ++i) {
        QPointF normal = curveNormal(pts, i, flipped);
        QPointF toRef = refPt - pts[i];
        double toRefLen = qSqrt(toRef.x() * toRef.x() + toRef.y() * toRef.y());
        if (toRefLen < 1e-12) {
            if (outNormal) *outNormal = normal;
            return i;
        }
        double dot = (normal.x() * toRef.x() + normal.y() * toRef.y()) / toRefLen;
        double absDot = qAbs(dot);
        if (absDot > bestVal) {
            bestVal = absDot;
            bestIdx = i;
            if (outNormal) *outNormal = normal;
        }
    }
    return bestIdx;
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
    bool flip1 = wf1->isNormalFlipped();
    bool flip2 = wf2->isNormalFlipped();
    int n = m_amountOfPoints;
    if (n < 2) n = 100;

    // Build SISL curves
    Geometry::SISLCurve curve1(wf1->controlPoints(), 3, true);
    Geometry::SISLCurve curve2(wf2->controlPoints(), 3, true);

    QVector<QPointF> pts1 = curve1.evaluateAll(n);
    QVector<QPointF> pts2 = curve2.evaluateAll(n);

    auto distSq = [](const QPointF& a, const QPointF& b) {
        double dx = a.x() - b.x();
        double dy = a.y() - b.y();
        return dx * dx + dy * dy;
    };

    // Step 1: Find indices on each WF where the normal best aligns with reference
    QPointF norm1Ref, norm2Ref;
    int ci1 = findBestNormalIndex(pts1, refPoint, flip1, &norm1Ref);
    int ci2 = findBestNormalIndex(pts2, refPoint, flip2, &norm2Ref);

    // Step 2: Compute signed distances to reference point
    //   If dot(normal, ref - wfPt) < 0, reference is behind the WF → negative
    QPointF toRef1 = refPoint - pts1[ci1];
    QPointF toRef2 = refPoint - pts2[ci2];
    double d1ref = qSqrt(toRef1.x() * toRef1.x() + toRef1.y() * toRef1.y());
    double d2ref = qSqrt(toRef2.x() * toRef2.x() + toRef2.y() * toRef2.y());

    double sign1 = (d1ref < 1e-12) ? 1.0 :
        ((norm1Ref.x() * toRef1.x() + norm1Ref.y() * toRef1.y()) / d1ref >= 0 ? 1.0 : -1.0);
    double sign2 = (d2ref < 1e-12) ? 1.0 :
        ((norm2Ref.x() * toRef2.x() + norm2Ref.y() * toRef2.y()) / d2ref >= 0 ? 1.0 : -1.0);

    // Optical path length constant (signed)
    double C = n1 * d1ref * sign1 + n2 * d2ref * sign2;

    auto* result = new CurveObject(resultName());
    result->setObjectType(withResult(ObjectType::Curve));
    result->setRefractiveIndex((n1 + n2) * 0.5);

    QVector<QPointF> ovalPts;
    ovalPts.reserve(n);

    // Step 3: For each sample pair on the curves, compute oval point X satisfying
    //   n1 * signed_d(X, p1) + n2 * signed_d(X, p2) = C
    // On the line X(t) = p1 + t*(p2-p1):
    //   signed_d(X, p1) = sign1 * t * L
    //   signed_d(X, p2) = sign2 * (1-t) * L
    // Equation: n1*sign1*t*L + n2*sign2*(1-t)*L = C
    //   L * (n2*sign2 + t*(n1*sign1 - n2*sign2)) = C
    //   t = (C/L - n2*sign2) / (n1*sign1 - n2*sign2)
    for (int i = 0; i < n; ++i) {
        int j = (ci2 + (i - ci1 + n) % n) % n;
        QPointF p1 = pts1[i];
        QPointF p2 = pts2[j];
        double L = qSqrt(distSq(p1, p2));

        if (L < 1e-9) {
            ovalPts.append((p1 + p2) * 0.5);
            continue;
        }

        // Get normals at these sample points for sign determination
        QPointF n1i = curveNormal(pts1, i, flip1);
        QPointF n2j = curveNormal(pts2, j, flip2);

        QPointF dir = p2 - p1;
        double Lsq = distSq(p1, p2);
        double Lval = qSqrt(Lsq);

        // Signs for this sample pair
        QPointF toP2 = p2 - p1;
        double s1 = (n1i.x() * toP2.x() + n1i.y() * toP2.y()) >= 0 ? 1.0 : -1.0;
        QPointF toP1 = p1 - p2;
        double s2 = (n2j.x() * toP1.x() + n2j.y() * toP1.y()) >= 0 ? 1.0 : -1.0;

        double C_L = C / Lval;
        double denom = n1 * s1 - n2 * s2;

        double t;
        if (qAbs(denom) < 1e-12) {
            t = 0.5;
        } else {
            t = (C_L - n2 * s2) / denom;
        }

        t = qBound(0.0, t, 1.0);
        ovalPts.append(p1 + dir * t);
    }

    // Force reference point onto the curve
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