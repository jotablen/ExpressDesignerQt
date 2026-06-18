#pragma once
#include <QString>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QMutex>

namespace ExpressDesigner {

enum class LogLevel { Trace, Info, Warn, Error };

class Logger {
public:
    static Logger& instance();
    void setFile(const QString& path);
    void log(LogLevel level, const QString& category, const QString& message);

    // Stack depth tracking
    void enter(const QString& category, const QString& function);
    void leave(const QString& category, const QString& function);

private:
    Logger() = default;
    ~Logger();
    QFile m_file;
    QTextStream m_out;
    QMutex m_mutex;
    int m_depth = 0;
    bool m_enabled = false;
    QString levelStr(LogLevel level) const;
};

// RAII scope tracer
class ScopeTracer {
public:
    ScopeTracer(const QString& category, const QString& function);
    ~ScopeTracer();
private:
    QString m_category, m_function;
};

} // namespace ExpressDesigner

// Convenience macros
#define LOG_TRACE(cat, msg) ExpressDesigner::Logger::instance().log(ExpressDesigner::LogLevel::Trace, cat, msg)
#define LOG_INFO(cat, msg)  ExpressDesigner::Logger::instance().log(ExpressDesigner::LogLevel::Info,  cat, msg)
#define LOG_WARN(cat, msg)  ExpressDesigner::Logger::instance().log(ExpressDesigner::LogLevel::Warn,  cat, msg)
#define LOG_ERROR(cat, msg) ExpressDesigner::Logger::instance().log(ExpressDesigner::LogLevel::Error, cat, msg)
#define TRACE_FUNC() ExpressDesigner::ScopeTracer __scopeTracer(QStringLiteral("UI"), QLatin1String(__FUNCTION__))
#define TRACE_CAT(cat) ExpressDesigner::ScopeTracer __scopeTracer(cat, QLatin1String(__FUNCTION__))
