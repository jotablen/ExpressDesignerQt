#include "Logger.h"
#include <QCoreApplication>
#include <QDir>
#include <QThread>
#include <QIODevice>

namespace ExpressDesigner {

Logger& Logger::instance()
{
    static Logger s;
    return s;
}

Logger::~Logger()
{
    if (m_file.isOpen())
        m_file.close();
}

void Logger::setFile(const QString& path)
{
    QMutexLocker lock(&m_mutex);
    if (m_file.isOpen())
        m_file.close();
    m_file.setFileName(path);
    if (m_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        m_out.setDevice(&m_file);
        m_enabled = true;
    }
}

QString Logger::levelStr(LogLevel level) const
{
    switch (level) {
    case LogLevel::Trace: return QStringLiteral("TRACE");
    case LogLevel::Info:  return QStringLiteral("INFO ");
    case LogLevel::Warn:  return QStringLiteral("WARN ");
    case LogLevel::Error: return QStringLiteral("ERROR");
    }
    return QStringLiteral("?????");
}

void Logger::enter(const QString& category, const QString& function)
{
    QMutexLocker lock(&m_mutex);
    if (!m_enabled) return;
    QString indent(m_depth * 2, QChar(' '));
    m_out << QDateTime::currentDateTime().toString(QStringLiteral("hh:mm:ss.zzz"))
          << " [" << levelStr(LogLevel::Trace) << "] "
          << "[" << category << "] "
          << indent << "-> " << function << Qt::endl;
    m_out.flush();
    ++m_depth;
}

void Logger::leave(const QString& category, const QString& function)
{
    QMutexLocker lock(&m_mutex);
    if (!m_enabled) return;
    --m_depth;
    if (m_depth < 0) m_depth = 0;
    QString indent(m_depth * 2, QChar(' '));
    m_out << QDateTime::currentDateTime().toString(QStringLiteral("hh:mm:ss.zzz"))
          << " [" << levelStr(LogLevel::Trace) << "] "
          << "[" << category << "] "
          << indent << "<- " << function << Qt::endl;
    m_out.flush();
}

void Logger::log(LogLevel level, const QString& category, const QString& message)
{
    QMutexLocker lock(&m_mutex);
    if (!m_enabled) return;
    QString indent(m_depth * 2, QChar(' '));
    m_out << QDateTime::currentDateTime().toString(QStringLiteral("hh:mm:ss.zzz"))
          << " [" << levelStr(level) << "] "
          << "[" << category << "] "
          << indent << message << Qt::endl;
    m_out.flush();
}

// ScopeTracer
ScopeTracer::ScopeTracer(const QString& category, const QString& function)
    : m_category(category), m_function(function)
{
    Logger::instance().enter(m_category, m_function);
}
ScopeTracer::~ScopeTracer()
{
    Logger::instance().leave(m_category, m_function);
}

} // namespace ExpressDesigner