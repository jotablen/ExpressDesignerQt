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

    // Create result curve with initial control points from WF1
    auto* result = new CurveObject(resultName());
    result->setObjectType(withResult(ObjectType::Curve));
    result->setRefractiveIndex(wf1->refractiveIndex());
    result->setControlPoints(wf1->controlPoints());
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