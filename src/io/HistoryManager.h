#pragma once

#include <QString>
#include <QVector>
#include <QJsonObject>
#include <QJsonArray>

namespace ExpressDesigner {

struct HistoryEntry {
    QString timestamp;
    QString action;
    QString detail;
};

class HistoryManager {
public:
    void addEntry(const QString& action, const QString& detail);
    void recordObjectCreation(const QString& objName);
    void recordObjectDeletion(const QString& objName);
    void recordObjectModification(const QString& objName, const QString& propertyName,
                                   const QString& oldValue, const QString& newValue);

    const QVector<HistoryEntry>& entries() const { return m_entries; }
    void clear() { m_entries.clear(); }

    QJsonObject toJson() const;
    void loadFromJson(const QJsonObject& json);

private:
    QVector<HistoryEntry> m_entries;
};

} // namespace ExpressDesigner