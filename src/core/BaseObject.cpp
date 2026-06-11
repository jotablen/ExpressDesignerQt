#include "BaseObject.h"

namespace ExpressDesigner {

BaseObject::BaseObject(ObjectType type, const QString& name, QObject* parent)
    : QObject(parent)
    , m_type(type)
    , m_name(name)
{
    if (m_name.isEmpty()) {
        m_name = QStringLiteral("Unnamed");
    }
    m_uuid = QUuid::createUuid();
}

BaseObject::~BaseObject() = default;

void BaseObject::setUuid(const QUuid& uuid)
{
    if (m_uuid != uuid) {
        m_uuid = uuid;
        emit objectModified(QStringLiteral("uuid"));
    }
}

QUuid BaseObject::uuid() const
{
    return m_uuid;
}

void BaseObject::setName(const QString& name)
{
    if (m_name != name) {
        m_name = name;
        emit nameChanged(m_name);
        emit objectModified(QStringLiteral("name"));
    }
}

QString BaseObject::name() const
{
    return m_name;
}

void BaseObject::setObjectType(ObjectType type)
{
    m_type = type;
    emit objectModified(QStringLiteral("type"));
}

ObjectType BaseObject::objectType() const
{
    return m_type;
}

bool BaseObject::isWavefront() const
{
    return ExpressDesigner::isWavefront(m_type);
}

bool BaseObject::isVirtualWF() const
{
    return ExpressDesigner::isVirtualWF(m_type);
}

bool BaseObject::isResult() const
{
    return ExpressDesigner::isResult(m_type);
}

bool BaseObject::isProject() const
{
    return (static_cast<uint16_t>(m_type) & static_cast<uint16_t>(ObjectType::Mask_Project)) != 0;
}

void BaseObject::setVisible(bool visible)
{
    if (m_visible != visible) {
        m_visible = visible;
        emit visibilityChanged(m_visible);
        emit objectModified(QStringLiteral("visible"));
    }
}

bool BaseObject::isVisible() const
{
    return m_visible;
}

void BaseObject::setLabelVisible(bool visible)
{
    if (m_showLabel != visible) {
        m_showLabel = visible;
        emit objectModified(QStringLiteral("labelVisible"));
    }
}

bool BaseObject::isLabelVisible() const
{
    return m_showLabel;
}

void BaseObject::setShowNormals(bool show)
{
    if (m_showNormals != show) {
        m_showNormals = show;
        emit objectModified(QStringLiteral("showNormals"));
    }
}

bool BaseObject::showNormals() const
{
    return m_showNormals;
}

void BaseObject::setNormalParams(int raysQty, double raysLen)
{
    m_raysQty = raysQty;
    m_raysLen = raysLen;
    emit objectModified(QStringLiteral("normalParams"));
}

int BaseObject::normalRaysQty() const
{
    return m_raysQty;
}

double BaseObject::normalRaysLen() const
{
    return m_raysLen;
}

void BaseObject::setNormalFlipped(bool flipped)
{
    if (m_normalFlipped != flipped) {
        m_normalFlipped = flipped;
        emit objectModified(QStringLiteral("normalFlipped"));
    }
}

bool BaseObject::isNormalFlipped() const
{
    return m_normalFlipped;
}

void BaseObject::flipNormal()
{
    setNormalFlipped(!m_normalFlipped);
}

void BaseObject::saveToJson(QJsonObject& json) const
{
    json[QStringLiteral("uuid")] = m_uuid.toString(QUuid::WithoutBraces);
    json[QStringLiteral("name")] = m_name;
    json[QStringLiteral("visible")] = m_visible;
    json[QStringLiteral("showLabel")] = m_showLabel;
    json[QStringLiteral("showNormals")] = m_showNormals;
    json[QStringLiteral("normalRaysQty")] = m_raysQty;
    json[QStringLiteral("normalRaysLen")] = m_raysLen;
    json[QStringLiteral("normalFlipped")] = m_normalFlipped;
    json[QStringLiteral("softwareVersion")] = m_softwareVersion;
}

void BaseObject::loadFromJson(const QJsonObject& json)
{
    m_uuid = QUuid::fromString(json[QStringLiteral("uuid")].toString());
    m_name = json[QStringLiteral("name")].toString();
    m_visible = json[QStringLiteral("visible")].toBool(true);
    m_showLabel = json[QStringLiteral("showLabel")].toBool(true);
    m_showNormals = json[QStringLiteral("showNormals")].toBool(false);
    m_raysQty = json[QStringLiteral("normalRaysQty")].toInt(10);
    m_raysLen = json[QStringLiteral("normalRaysLen")].toDouble(1.0);
    m_normalFlipped = json[QStringLiteral("normalFlipped")].toBool(false);
    m_softwareVersion = json[QStringLiteral("softwareVersion")].toInt(4);
}

} // namespace ExpressDesigner