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

    // Interpolate control points between the two WFs
    const auto& pts1 = wf1->controlPoints();
    const auto& pts2 = wf2->controlPoints();
    int maxCount = qMax(pts1.size(), pts2.size());
    QVector<QPointF> ovalPts;
    ovalPts.reserve(maxCount);
    for (int i = 0; i < maxCount; ++i) {
        double t = (maxCount > 1) ? static_cast<double>(i) / (maxCount - 1) : 0.0;
        int idx1 = qMin(i, pts1.size() - 1);
        int idx2 = qMin(i, pts2.size() - 1);
        QPointF p1 = pts1.value(idx1, QPointF());
        QPointF p2 = pts2.value(idx2, QPointF());
        // Midpoint between corresponding WF points creates the oval
        ovalPts.append(QPointF((p1.x() + p2.x()) * 0.5, (p1.y() + p2.y()) * 0.5));
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
    return m_name + QStringLiteral("_OvalResult");
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