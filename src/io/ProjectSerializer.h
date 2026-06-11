#pragma once

#include <QString>

namespace ExpressDesigner {

class Project;
class HistoryManager;

class ProjectSerializer {
public:
    static bool save(const Project& project, const QString& filePath);
    static Project* load(const QString& filePath, QObject* parent = nullptr);
    static bool saveHistory(const HistoryManager& history, const QString& filePath);
    static void loadHistory(HistoryManager& history, const QString& filePath);
};

} // namespace ExpressDesigner