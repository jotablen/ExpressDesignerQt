#include "CarthesianOvalOperation.h"
#include "Project.h"
#include "CurveObject.h"
#include <geometry/SISLWrapper.h>
#include <QtMath>
#include <cmath>

namespace ExpressDesigner {

CarthesianOvalOperation::CarthesianOvalOperation(const QString& name, QObject* parent)
    : CustomOperation(OperationType::CartesianOval, name, parent)
{
    m_paramNames << QStringLiteral("WF1") << QStringLiteral("WF2")
                 << QStringLiteral("ReferencePoint") << QStringLiteral("Result");
}

CarthesianOvalOperation::~CarthesianOvalOperation() = default;

// CheckGoodReferencePoint: verify there is at least one sampled point on the
// interpolated curve whose normal vector points toward / passes through the reference point.
// Returns true if such a point exists and stores the index.
static bool checkGoodReferencePoint(const QVector<QPointF>& curvePoints,
                                     const QPointF& refPoint,
                                     int* goodIndex = nullptr)
{
    // Build a SISL curve from the discrete points and evaluate normals along it
    if (curvePoints.size() < 2) return false;

    // Sample finely along the curve
    int samples = qMax(curvePoints.size() * 3, 100);
    Geometry::SISLCurve sislCurve(curvePoints, 3, true);
    auto sampled = sislCurve.evaluateAll(samples);

    double bestAngle = 1e18;
    int bestIdx = 0;

    for (int i = 0; i < samples; ++i) {
        // Tangent direction
        int next = (i + 1) % samples;
        QPointF dir = sampled[next] - sampled[i];
        double len = qSqrt(dir.x() * dir.x() + dir.y() * dir.y());
        if (len < 1e-12) continue;

        // Normal (perpendicular to tangent)
        QPointF normal(-dir.y() / len, dir.x() / len);

        // Vector from sampled point to reference point
        QPointF toRef = refPoint - sampled[i];
        double toRefLen = qSqrt(toRef.x() * toRef.x() + toRef.y() * toRef.y());
        if (toRefLen < 1e-12) {
            // Reference point coincides with a curve point
            if (goodIndex) *goodIndex = i;
            return true;
        }

        // Angle between normal and vector-to-reference
        double dot = normal.x() * toRef.x() / toRefLen + normal.y() * toRef.y() / toRefLen;
        double angle = qAcos(qBound(-1.0, dot, 1.0));
        // Also check flipped normal
        double dotFlipped = -normal.x() * toRef.x() / toRefLen - normal.y() * toRef.y() / toRefLen;
        double angleFlipped = qAcos(qBound(-1.0, dotFlipped, 1.0));
        double minAngle = qMin(angle, angleFlipped);

        if (minAngle < bestAngle) {
            bestAngle = minAngle;
            bestIdx = i;
        }
    }

    // Consider it "good" if angle < ~15 degrees
    if (bestAngle < 0.2618) { // ~15 degrees
        if (goodIndex) *goodIndex = bestIdx;
        return true;
    }
    return false;
}

bool CarthesianOvalOperation::execute(Project* project)
{
    m_errorCode = 0;
    if (!project) {
        m_errorCode = -1;
        m_errorMessage = QStringLiteral("No project provided");
        return false;
    }

    // Find named objects in project
    CustomObject* wf1 = project->findObject(m_paramNames.value(PARAM_WF1));
    CustomObject* wf2 = project->findObject(m_paramNames.value(PARAM_WF2));
    if (!wf1 || !wf2) {
        m_errorCode = -2;
        m_errorMessage = QStringLiteral("Wavefront objects not found");
        return false;
    }

    // Find reference point object
    CustomObject* refObj = project->findObject(m_paramNames.value(PARAM_REF_POINT));
    QPointF refPoint(0.0, 0.0);
    if (refObj && !refObj->controlPoints().isEmpty())
        refPoint = refObj->controlPoints().first();

    double n1 = wf1->refractiveIndex();
    double n2 = wf2->refractiveIndex();
    int n = m_amountOfPoints;
    if (n < 2) n = 100;

    // Build SISL curves from both wavefronts (operate against curves, not point lists)
    Geometry::SISLCurve curve1(wf1->controlPoints(), 3, wf1->isWavefront() ? /*open=*/true : true);
    Geometry::SISLCurve curve2(wf2->controlPoints(), 3, wf2->isWavefront() ? true : true);

    // Sample both curves uniformly
    QVector<QPointF> pts1 = curve1.evaluateAll(n);
    QVector<QPointF> pts2 = curve2.evaluateAll(n);

    // Helper: squared distance
    auto distSq = [](const QPointF& a, const QPointF& b) {
        double dx = a.x() - b.x();
        double dy = a.y() - b.y();
        return dx * dx + dy * dy;
    };

    // Check that the reference point is "good" — it should align with a curve point's normal
    // Check against the closer WF first
    bool isGoodRef = checkGoodReferencePoint(pts1, refPoint, nullptr) ||
                     checkGoodReferencePoint(pts2, refPoint, nullptr);

    // Find closest points on each sampled curve to the reference point
    int ci1 = 0, ci2 = 0;
    double bestD1 = 1e18, bestD2 = 1e18;
    for (int i = 0; i < n; ++i) {
        double d1 = distSq(pts1[i], refPoint);
        double d2 = distSq(pts2[i], refPoint);
        if (d1 < bestD1) { bestD1 = d1; ci1 = i; }
        if (d2 < bestD2) { bestD2 = d2; ci2 = i; }
    }

    // Compute the reference optical path length constant C = n1 * d1ref + n2 * d2ref
    double d1ref = qSqrt(bestD1);
    double d2ref = qSqrt(bestD2);
    double C = n1 * d1ref + n2 * d2ref;
    if (C < 1e-12 && !isGoodRef) {
        m_errorCode = -3;
        m_errorMessage = QStringLiteral("Optical path length constant is zero — check reference point");
        return false;
    }

    // If C is zero but reference is good, use minimum non-zero OPL from any pair
    if (C < 1e-12) {
        for (int i = 0; i < n; ++i) {
            double d1 = qSqrt(distSq(pts1[i], refPoint));
            double d2 = qSqrt(distSq(pts2[i], refPoint));
            double val = n1 * d1 + n2 * d2;
            if (val > 1e-12) {
                C = val;
                break;
            }
        }
    }

    // Create result curve — a Cartesian oval
    auto* result = new CurveObject(resultName());
    result->setObjectType(withResult(ObjectType::Curve));
    result->setRefractiveIndex((n1 + n2) * 0.5);

    QVector<QPointF> ovalPts;
    ovalPts.reserve(n);

    // For each pair of corresponding sampled points on the two curves,
    // find the point X on the line p1→p2 that satisfies n1*|X-p1| + n2*|X-p2| = C
    for (int i = 0; i < n; ++i) {
        QPointF p1 = pts1[i];
        // Map WF2 index relative to reference offset
        int j = (ci2 + (i - ci1 + n) % n) % n;

        QPointF p2 = pts2[j];
        double L = qSqrt(distSq(p1, p2));

        if (L < 1e-9) {
            // WF points coincide at this sample — use midpoint
            ovalPts.append((p1 + p2) * 0.5);
            continue;
        }

        // On the line X(t) = p1 + t*(p2-p1), t∈[0,1]:
        //   d(X,p1) = t*L,  d(X,p2) = (1-t)*L
        //   f(t) = n1*t*L + n2*(1-t)*L = L * (n2 + t*(n1-n2))
        // Setting f(t) = C gives:
        //   t = (C/L - n2) / (n1 - n2)
        double Cnorm = C / L;
        double denom = n1 - n2;

        double t;
        if (qAbs(denom) < 1e-12) {
            // n1 == n2: constant OPL regardless of t; place at midpoint
            t = 0.5;
        } else {
            t = (Cnorm - n2) / denom;
        }

        // Clamp t to [0, 1] to stay between the wavefronts
        t = qBound(0.0, t, 1.0);

        ovalPts.append(p1 + (p2 - p1) * t);
    }

    // Ensure the reference point is part of the curve —
    // replace the point closest to the reference with the reference itself
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

QString CarthesianOvalOperation::resultName() const
{
    return m_name;
}

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