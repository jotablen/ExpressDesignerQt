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

// Check if the point pto_WF on a wavefront has its normal aligned with the
// direction toward the reference point (IsGoodClosestPoint).
static bool isGoodClosestPoint(const Geometry::SISLCurve& curve,
                                double t, bool flipped,
                                const QPointF& wfPt, const QPointF& refPt)
{
    QPointF normal = curve.normal(t, flipped);
    QPointF toRef = refPt - wfPt;
    double toRefLen = qSqrt(toRef.x() * toRef.x() + toRef.y() * toRef.y());
    if (toRefLen < 1e-12) return true;

    double dot = (normal.x() * toRef.x() + normal.y() * toRef.y()) / toRefLen;
    double absDot = qAbs(dot);
    // Accept if cos(theta) >= ~0.9999 (angle < ~1 degree)
    return qAbs(absDot - 1.0) < 5.0e-5;
}

// Find the sample index on a curve whose normal best aligns with the reference point
static int findBestNormalIndex(const Geometry::SISLCurve& curve,
                                int numSamples, bool flipped,
                                const QVector<QPointF>& sampledPts,
                                const QPointF& refPt,
                                QPointF* outNormal = nullptr)
{
    double bestAbsDot = -1.0;
    int bestIdx = 0;
    for (int i = 0; i < numSamples; ++i) {
        double t = numSamples > 1 ? static_cast<double>(i) / (numSamples - 1) : 0.0;
        QPointF normal = curve.normal(t, flipped);
        QPointF toRef = refPt - sampledPts[i];
        double toRefLen = qSqrt(toRef.x() * toRef.x() + toRef.y() * toRef.y());
        if (toRefLen < 1e-12) {
            if (outNormal) *outNormal = normal;
            return i;
        }
        double dot = (normal.x() * toRef.x() + normal.y() * toRef.y()) / toRefLen;
        double absDot = qAbs(dot);
        if (absDot > bestAbsDot) {
            bestAbsDot = absDot;
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

    // Build SISL curves — use continuous derivative for normals
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
    int ci1 = findBestNormalIndex(curve1, n, flip1, pts1, refPoint, &norm1Ref);
    int ci2 = findBestNormalIndex(curve2, n, flip2, pts2, refPoint, &norm2Ref);

    // Step 2: Compute signed distances to reference point
    QPointF toRef1 = refPoint - pts1[ci1];
    QPointF toRef2 = refPoint - pts2[ci2];
    double d1ref = qSqrt(toRef1.x() * toRef1.x() + toRef1.y() * toRef1.y());
    double d2ref = qSqrt(toRef2.x() * toRef2.x() + toRef2.y() * toRef2.y());

    double sign1 = (d1ref < 1e-12) ? 1.0 :
        ((norm1Ref.x() * toRef1.x() + norm1Ref.y() * toRef1.y()) / d1ref >= 0 ? 1.0 : -1.0);
    double sign2 = (d2ref < 1e-12) ? 1.0 :
        ((norm2Ref.x() * toRef2.x() + norm2Ref.y() * toRef2.y()) / d2ref >= 0 ? 1.0 : -1.0);

    // Signed optical path length constant
    double C = n1 * d1ref * sign1 + n2 * d2ref * sign2;

    auto* result = new CurveObject(resultName());
    result->setObjectType(withResult(ObjectType::Curve));
    result->setRefractiveIndex((n1 + n2) * 0.5);

    QVector<QPointF> ovalPts;
    ovalPts.reserve(n);

    // Step 3: For each sample pair, compute oval point via signed OPL equation
    for (int i = 0; i < n; ++i) {
        int j = (ci2 + (i - ci1 + n) % n) % n;
        QPointF p1 = pts1[i];
        QPointF p2 = pts2[j];
        double L = qSqrt(distSq(p1, p2));
        if (L < 1e-9) {
            ovalPts.append((p1 + p2) * 0.5);
            continue;
        }

        // Normals at this sample pair via SISLCurve::normal()
        double t1 = n > 1 ? static_cast<double>(i) / (n - 1) : 0.0;
        double t2 = n > 1 ? static_cast<double>(j) / (n - 1) : 0.0;
        QPointF n1i = curve1.normal(t1, flip1);
        QPointF n2j = curve2.normal(t2, flip2);

        QPointF dir = p2 - p1;
        double s1 = (n1i.x() * dir.x() + n1i.y() * dir.y()) >= 0 ? 1.0 : -1.0;
        QPointF revDir = p1 - p2;
        double s2 = (n2j.x() * revDir.x() + n2j.y() * revDir.y()) >= 0 ? 1.0 : -1.0;

        double C_L = C / L;
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