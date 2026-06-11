#include "HistoryManager.h"
#include <QDateTime>

namespace ExpressDesigner {

void HistoryManager::addEntry(const QString& action, const QString& detail)
{
    HistoryEntry entry;
    entry.timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
    entry.action = action;
    entry.detail = detail;
    m_entries.append(entry);
}

void HistoryManager::recordObjectCreation(const QString& objName)
{
    addEntry(QStringLiteral("object_created"), objName);
}

void HistoryManager::recordObjectDeletion(const QString& objName)
{
    addEntry(QStringLiteral("object_deleted"), objName);
}

void HistoryManager::recordObjectModification(const QString& objName, const QString& propertyName,
                                               const QString& oldValue, const QString& newValue)
{
    addEntry(QStringLiteral("object_modified"),
             QStringLiteral("%1: %2 changed from %3 to %4").arg(objName, propertyName, oldValue, newValue));
}

QJsonObject HistoryManager::toJson() const
{
    QJsonArray arr;
    for (const auto& entry : m_entries) {
        QJsonObject e;
        e[QStringLiteral("timestamp")] = entry.timestamp;
        e[QStringLiteral("action")] = entry.action;
        e[QStringLiteral("detail")] = entry.detail;
        arr.append(e);
    }
    QJsonObject root;
    root[QStringLiteral("history")] = arr;
    return root;
}

void HistoryManager::loadFromJson(const QJsonObject& json)
{
    m_entries.clear();
    for (const auto& val : json[QStringLiteral("history")].toArray()) {
        QJsonObject e = val.toObject();
        HistoryEntry entry;
        entry.timestamp = e[QStringLiteral("timestamp")].toString();
        entry.action = e[QStringLiteral("action")].toString();
        entry.detail = e[QStringLiteral("detail")].toString();
        m_entries.append(entry);
    }
}

} // namespace ExpressDesigner