#include "CustomOperation.h"
#include <QJsonArray>

namespace ExpressDesigner {

CustomOperation::CustomOperation(OperationType opType, const QString& name, QObject* parent)
    : BaseObject(ObjectType::Mask_Project, name, parent)
    , m_opType(opType)
{
}

CustomOperation::~CustomOperation() = default;

OperationType CustomOperation::operationType() const { return m_opType; }

void CustomOperation::setAmountOfPoints(int points) { m_amountOfPoints = qMax(points, 2); }
int CustomOperation::amountOfPoints() const { return m_amountOfPoints; }

const QStringList& CustomOperation::paramNames() const { return m_paramNames; }
void CustomOperation::addParamName(const QString& name) { m_paramNames.append(name); }
int CustomOperation::paramCount() const { return m_paramNames.size(); }
QString CustomOperation::paramName(int index) const { return m_paramNames.value(index); }
void CustomOperation::setParamName(int index, const QString& name)
{
    if (index >= 0 && index < m_paramNames.size())
        m_paramNames[index] = name;
}
bool CustomOperation::isNameInParams(const QString& name) const
{
    return m_paramNames.contains(name);
}

bool CustomOperation::execute(Project*)
{
    m_errorCode = 0;
    m_errorMessage.clear();
    return true;
}

bool CustomOperation::isParamObject(int) const { return false; }

QString CustomOperation::paramPrefixOnTree(int index) const
{
    return QStringLiteral("Param%1").arg(index + 1);
}

int CustomOperation::errorCode() const { return m_errorCode; }
QString CustomOperation::errorMessage() const { return m_errorMessage; }

void CustomOperation::saveToJson(QJsonObject& json) const
{
    BaseObject::saveToJson(json);
    json[QStringLiteral("operationType")] = static_cast<int>(m_opType);
    json[QStringLiteral("amountOfPoints")] = m_amountOfPoints;

    QJsonArray arr;
    for (const auto& name : m_paramNames) arr.append(name);
    json[QStringLiteral("paramNames")] = arr;
}

void CustomOperation::loadFromJson(const QJsonObject& json)
{
    BaseObject::loadFromJson(json);
    m_opType = static_cast<OperationType>(json[QStringLiteral("operationType")].toInt());
    m_amountOfPoints = json[QStringLiteral("amountOfPoints")].toInt(100);
    m_paramNames.clear();
    for (const auto& v : json[QStringLiteral("paramNames")].toArray())
        m_paramNames.append(v.toString());
}

} // namespace ExpressDesigner