#include "CarthesianOvalOperation.h"
#include "Project.h"
#include "CurveObject.h"
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

    const auto& pts1 = wf1->controlPoints();
    const auto& pts2 = wf2->controlPoints();
    int n = m_amountOfPoints;
    if (n < 2) n = 100;

    // Helper: squared distance
    auto distSq = [](const QPointF& a, const QPointF& b) {
        double dx = a.x() - b.x();
        double dy = a.y() - b.y();
        return dx * dx + dy * dy;
    };

    // Helper: linear resample of control points to n evenly-spaced points
    auto resample = [](const QVector<QPointF>& src, int count) -> QVector<QPointF> {
        if (src.size() < 2 || count < 2) return src;
        QVector<QPointF> dst;
        dst.reserve(count);
        double totalLen = 0;
        QVector<double> segLen;
        segLen.reserve(src.size() - 1);
        for (int i = 1; i < src.size(); ++i) {
            double d = std::hypot(src[i].x() - src[i-1].x(), src[i].y() - src[i-1].y());
            segLen.append(d);
            totalLen += d;
        }
        if (totalLen < 1e-9) return src;
        double step = totalLen / (count - 1);
        int seg = 0;
        double accum = 0;
        for (int i = 0; i < count; ++i) {
            double target = i * step;
            while (seg < segLen.size() && accum + segLen[seg] < target) {
                accum += segLen[seg];
                ++seg;
            }
            if (seg >= segLen.size()) {
                dst.append(src.last());
            } else {
                double t = (target - accum) / segLen[seg];
                dst.append(src[seg] + (src[seg+1] - src[seg]) * t);
            }
        }
        return dst;
    };

    auto r1 = resample(pts1, n);
    auto r2 = resample(pts2, n);

    // Find closest points on each WF to the reference point
    int ci1 = 0, ci2 = 0;
    double bestD1 = 1e18, bestD2 = 1e18;
    for (int i = 0; i < n; ++i) {
        double d1 = distSq(r1[i], refPoint);
        double d2 = distSq(r2[i], refPoint);
        if (d1 < bestD1) { bestD1 = d1; ci1 = i; }
        if (d2 < bestD2) { bestD2 = d2; ci2 = i; }
    }

    // Compute the reference optical path length constant C
    double d1ref = qSqrt(bestD1);
    double d2ref = qSqrt(bestD2);
    double C = n1 * d1ref + n2 * d2ref;
    if (C < 1e-12) {
        m_errorCode = -3;
        m_errorMessage = QStringLiteral("Optical path length constant is zero — check reference point");
        return false;
    }

    // Create result curve — a Cartesian oval
    auto* result = new CurveObject(resultName());
    result->setObjectType(withResult(ObjectType::Curve));
    result->setRefractiveIndex((n1 + n2) * 0.5);

    // Compute oval points: for each pair of corresponding WF points,
    // find the point X on the line p1→p2 satisfying n1*|X-p1| + n2*|X-p2| = C
    QVector<QPointF> ovalPts;
    ovalPts.reserve(n);

    // Rotate indices so the reference pair (ci1, ci2) is at index 0,
    // making the oval start near the reference point
    int offset = ci1;
    for (int k = 0; k < n; ++k) {
        int i = (k + offset) % n;
        // Match WF2 index proportionally (or use closest approach)
        // Map i proportionally, but also account for the reference shift
        int j = (ci2 + (i - ci1 + n) % n) % n;
        // Ensure j is valid
        if (j < 0) j = 0;
        if (j >= n) j = n - 1;

        QPointF p1 = r1[i];
        QPointF p2 = r2[j];
        double L = qSqrt(distSq(p1, p2));

        if (L < 1e-9) {
            // WF points coincide — use reference point
            ovalPts.append(refPoint);
            continue;
        }

        // On the line X(t) = p1 + t*(p2-p1), t∈[0,1]:
        //   d(X,p1) = t*L,  d(X,p2) = (1-t)*L
        //   f(t) = n1*t*L + n2*(1-t)*L = L*(n2 + t*(n1-n2))
        // Setting f(t) = C:
        //   t = (C/L - n2) / (n1 - n2)
        double Cnorm = C / L;
        double denom = n1 - n2;

        double t;
        if (qAbs(denom) < 1e-12) {
            // n1 == n2: f(t) = n1*L (constant). If C matches, any t works; pick 0.5
            t = (qAbs(Cnorm - n1) < 1e-9) ? 0.5 : 0.5;
        } else {
            t = (Cnorm - n2) / denom;
        }

        // Clamp t to [0, 1] for points between the two WFs
        t = qBound(0.0, t, 1.0);

        QPointF ovalPt = p1 + (p2 - p1) * t;
        ovalPts.append(ovalPt);
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