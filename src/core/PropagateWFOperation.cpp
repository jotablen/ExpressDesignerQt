#include "PropagateWFOperation.h"
#include "Project.h"
#include "CurveObject.h"

namespace ExpressDesigner {

PropagateWFOperation::PropagateWFOperation(const QString& name, QObject* parent)
    : CustomOperation(OperationType::PropagateWF, name, parent)
{
    m_paramNames << QStringLiteral("WF") << QStringLiteral("Surface")
                 << QStringLiteral("IOR") << QStringLiteral("Offset")
                 << QStringLiteral("Result");
}

PropagateWFOperation::~PropagateWFOperation() = default;

bool PropagateWFOperation::execute(Project* project)
{
    m_errorCode = 0;
    if (!project) {
        m_errorCode = -1;
        return false;
    }

    CustomObject* wf = project->findObject(m_paramNames.value(PARAM_WF));
    CustomObject* surface = project->findObject(m_paramNames.value(PARAM_SURFACE));
    if (!wf || !surface) {
        m_errorCode = -2;
        return false;
    }

    auto* result = new CurveObject(resultName());
    result->setObjectType(withResult(ObjectType::Curve));
    project->addResultObject(result);

    emit operationExecuted(true);
    return true;
}

bool PropagateWFOperation::isParamObject(int index) const
{
    return index == PARAM_WF || index == PARAM_SURFACE;
}

QString PropagateWFOperation::resultName() const
{
    return m_name + QStringLiteral("_Propagated");
}

QString PropagateWFOperation::paramPrefixOnTree(int index) const
{
    switch (index) {
    case PARAM_WF: return QStringLiteral("WF: ");
    case PARAM_SURFACE: return QStringLiteral("Srf: ");
    case PARAM_IOR: return QStringLiteral("IOR: ");
    case PARAM_OFFSET: return QStringLiteral("Off: ");
    case PARAM_RESULT: return QStringLiteral("Res: ");
    default: return CustomOperation::paramPrefixOnTree(index);
    }
}

double PropagateWFOperation::offset() const { return m_offset; }
void PropagateWFOperation::setOffset(double value) { m_offset = value; }

void PropagateWFOperation::saveToJson(QJsonObject& json) const
{
    CustomOperation::saveToJson(json);
    json[QStringLiteral("operationSubType")] = QStringLiteral("propagateWF");
    json[QStringLiteral("offset")] = m_offset;
}

void PropagateWFOperation::loadFromJson(const QJsonObject& json)
{
    CustomOperation::loadFromJson(json);
    m_offset = json[QStringLiteral("offset")].toDouble(0.0);
}

} // namespace ExpressDesigner