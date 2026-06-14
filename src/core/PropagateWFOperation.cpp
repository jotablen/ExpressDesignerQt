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
    result->setObjectType(withWavefront(withResult(ObjectType::Curve)));
    result->setRefractiveIndex(m_paramNames.value(PARAM_IOR).toDouble());

    // Inherit normal display parameters from the source WF
    result->setNormalParams(wf->normalRaysQty(), wf->normalRaysLen());
    if (wf->isNormalFlipped())
        result->flipNormal();
    // Offset the WF control points along the surface normal direction (simplified: radial offset)
    QVector<QPointF> propagatedPts;
    const auto& wfPts = wf->controlPoints();
    const auto& surfPts = surface->controlPoints();
    double offset = m_offset;
    propagatedPts.reserve(qMax(wfPts.size(), surfPts.size()));
    int maxCount = qMax(wfPts.size(), surfPts.size());
    for (int i = 0; i < maxCount; ++i) {
        int idxWf = qMin(i, wfPts.size() - 1);
        int idxSurf = qMin(i, surfPts.size() - 1);
        QPointF pw = wfPts.value(idxWf, QPointF());
        QPointF ps = surfPts.value(idxSurf, QPointF());
        // Propagate WF point toward surface point by the offset distance
        QPointF dir = ps - pw;
        double len = std::sqrt(dir.x() * dir.x() + dir.y() * dir.y());
        if (len > 1e-9)
            dir /= len;
        else
            dir = QPointF(1, 0);
        propagatedPts.append(pw + dir * offset);
    }
    result->setControlPoints(propagatedPts);
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
    return m_name;
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