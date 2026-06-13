#include "CarthesianOvalOperation.h"
#include "Project.h"
#include "CurveObject.h"

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

    // Create result curve — interpolate between both WFs for a visibly distinct oval
    auto* result = new CurveObject(resultName());
    result->setObjectType(withResult(ObjectType::Curve));
    // Use average refractive index
    result->setRefractiveIndex((wf1->refractiveIndex() + wf2->refractiveIndex()) * 0.5);

    // Resample both WFs to a common point count (amountOfPoints) before interpolation
    const auto& pts1 = wf1->controlPoints();
    const auto& pts2 = wf2->controlPoints();
    int n = m_amountOfPoints;
    if (n < 2) n = 100;

    // Helper: linear resample of control points to n evenly-spaced points
    auto resample = [](const QVector<QPointF>& src, int count) -> QVector<QPointF> {
        if (src.size() < 2 || count < 2) return src;
        QVector<QPointF> dst;
        dst.reserve(count);
        double totalLen = 0;
        QVector<double> segLen;
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
    QVector<QPointF> ovalPts;
    ovalPts.reserve(n);
    for (int i = 0; i < n; ++i) {
        ovalPts.append(QPointF((r1[i].x() + r2[i].x()) * 0.5,
                                (r1[i].y() + r2[i].y()) * 0.5));
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