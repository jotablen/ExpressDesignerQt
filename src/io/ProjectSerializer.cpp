#include "ProjectSerializer.h"
#include "BaseObject.h"
#include "CustomObject.h"
#include "CustomOperation.h"
#include "ObjectFactory.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

namespace ExpressDesigner {

bool ProjectSerializer::save(const Project& project, const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QJsonObject root;
    root[QStringLiteral("formatVersion")] = QStringLiteral("2.0");
    root[QStringLiteral("software")] = QStringLiteral("ExpressDesigner");
    root[QStringLiteral("softwareVersion")] = QStringLiteral("4.0.0");
    QJsonObject projJson;
    project.saveToJson(projJson);
    root[QStringLiteral("project")] = projJson;
    QJsonDocument doc(root);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

Project* ProjectSerializer::load(const QString& filePath, QObject* parent)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return nullptr;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    QJsonObject root = doc.object();
    QJsonObject projJson = root[QStringLiteral("project")].toObject();
    auto* project = new Project(projJson[QStringLiteral("name")].toString(), parent);
    project->loadFromJson(projJson);
    return project;
}

bool ProjectSerializer::saveHistory(const HistoryManager& history, const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QJsonDocument doc(history.toJson());
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

void ProjectSerializer::loadHistory(HistoryManager& history, const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    history.loadFromJson(doc.object());
}

} // namespace ExpressDesigner