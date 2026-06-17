#include "CommandHistory.h"
#include "CustomObject.h"
#include "CustomOperation.h"
#include "Project.h"
#include <QtMath>
#include <QVariant>

namespace ExpressDesigner {

// ============================================================================
// Command base
// ============================================================================
Command::Command(const QString& description)
    : m_description(description), m_timestamp(QDateTime::currentDateTime()) {}
Command::~Command() = default;
QString Command::description() const { return m_description; }
QDateTime Command::timestamp() const { return m_timestamp; }

// ============================================================================
// AddObjectCommand
// ============================================================================
AddObjectCommand::AddObjectCommand(CustomObject* obj)
    : Command(QStringLiteral("Add ") + (obj ? obj->name() : QString())), m_obj(obj) {}

bool AddObjectCommand::execute(Project* project) {
    if (!project || !m_obj) return false;
    if (m_obj->isResult()) { project->addResultObject(m_obj); }
    else { project->addDataObject(m_obj); }
    return true;
}
bool AddObjectCommand::undo(Project* project) {
    if (!project || !m_obj) return false;
    m_wasResult = m_obj->isResult();
    if (m_wasResult) { project->removeResultObject(m_obj); }
    else { project->removeDataObject(m_obj); }
    return true;
}
CustomObject* AddObjectCommand::object() const { return m_obj; }

// ============================================================================
// DeleteObjectCommand
// ============================================================================
DeleteObjectCommand::DeleteObjectCommand(CustomObject* obj, bool isResult)
    : Command(QStringLiteral("Delete ") + (obj ? obj->name() : QString()))
    , m_obj(obj), m_isResult(isResult) {}

bool DeleteObjectCommand::execute(Project* project) {
    if (!project || !m_obj) return false;
    m_index = project->findObjectIndex(m_obj);
    if (m_isResult) project->removeResultObject(m_obj);
    else project->removeDataObject(m_obj);
    return true;
}
bool DeleteObjectCommand::undo(Project* project) {
    if (!project || !m_obj) return false;
    if (m_isResult) project->addResultObject(m_obj);
    else project->addDataObject(m_obj);
    return true;
}

// ============================================================================
// ModifyObjectCommand
// ============================================================================
ModifyObjectCommand::ModifyObjectCommand(CustomObject* obj, const QString& prop,
                                         const QVariant& oldVal, const QVariant& newVal)
    : Command(QStringLiteral("Modify %1").arg(obj ? obj->name() : QString()))
    , m_obj(obj), m_property(prop), m_oldValue(oldVal), m_newValue(newVal) {}

bool ModifyObjectCommand::execute(Project* project) {
    Q_UNUSED(project);
    if (!m_obj) return false;
    if (m_property == QStringLiteral("name")) m_obj->setName(m_newValue.toString());
    else if (m_property == QStringLiteral("refractiveIndex")) m_obj->setRefractiveIndex(m_newValue.toDouble());
    else if (m_property == QStringLiteral("visible")) m_obj->setVisible(m_newValue.toBool());
    else if (m_property == QStringLiteral("normalFlipped")) m_obj->setNormalFlipped(m_newValue.toBool());
    return true;
}
bool ModifyObjectCommand::undo(Project* project) {
    Q_UNUSED(project);
    if (!m_obj) return false;
    if (m_property == QStringLiteral("name")) m_obj->setName(m_oldValue.toString());
    else if (m_property == QStringLiteral("refractiveIndex")) m_obj->setRefractiveIndex(m_oldValue.toDouble());
    else if (m_property == QStringLiteral("visible")) m_obj->setVisible(m_oldValue.toBool());
    else if (m_property == QStringLiteral("normalFlipped")) m_obj->setNormalFlipped(m_oldValue.toBool());
    return true;
}

// ============================================================================
// ModifyControlPointsCommand
// ============================================================================
ModifyControlPointsCommand::ModifyControlPointsCommand(CustomObject* obj,
                                                       const QVector<QPointF>& oldPts,
                                                       const QVector<QPointF>& newPts)
    : Command(QStringLiteral("Edit points of ") + (obj ? obj->name() : QString()))
    , m_obj(obj), m_oldPoints(oldPts), m_newPoints(newPts) {}

bool ModifyControlPointsCommand::execute(Project* project) {
    Q_UNUSED(project);
    if (!m_obj) return false;
    m_obj->setControlPoints(m_newPoints);
    return true;
}
bool ModifyControlPointsCommand::undo(Project* project) {
    Q_UNUSED(project);
    if (!m_obj) return false;
    m_obj->setControlPoints(m_oldPoints);
    return true;
}

// ============================================================================
// ExecuteOperationCommand
// ============================================================================
ExecuteOperationCommand::ExecuteOperationCommand(CustomOperation* op)
    : Command(QStringLiteral("Execute ") + (op ? op->name() : QString())), m_op(op) {}

bool ExecuteOperationCommand::execute(Project* project) {
    if (!project || !m_op) return false;
    QString resultName = m_op->resultName();
    m_resultObj = project->findObject(resultName);
    if (m_resultObj) {
        project->removeResultObject(m_resultObj);
        project->removeDataObject(m_resultObj);
        m_op->execute(project);
        m_resultObj = project->findObject(resultName);
        return true;
    }
    // First execution
    if (!m_op->execute(project)) return false;
    m_resultObj = project->findObject(resultName);
    m_wasAdded = true;
    return true;
}
bool ExecuteOperationCommand::undo(Project* project) {
    if (!project || !m_resultObj || !m_wasAdded) return false;
    project->removeResultObject(m_resultObj);
    project->removeOperation(m_op);
    return true;
}
CustomOperation* ExecuteOperationCommand::operation() const { return m_op; }
CustomObject* ExecuteOperationCommand::resultObject() const { return m_resultObj; }

// ============================================================================
// RotateObjectCommand
// ============================================================================
RotateObjectCommand::RotateObjectCommand(CustomObject* obj, double degrees, PivotMode pivot)
    : Command(QStringLiteral("Rotate ") + (obj ? obj->name() : QString()))
    , m_obj(obj), m_degrees(degrees), m_pivot(pivot) {}

bool RotateObjectCommand::execute(Project* project) {
    Q_UNUSED(project);
    if (!m_obj) return false;
    const QVector<QPointF>& pts = m_obj->controlPoints();
    if (pts.isEmpty()) return false;
    m_oldPoints = pts;

    // Determine pivot
    switch (m_pivot) {
    case StartPoint: m_pivotPoint = pts.first(); break;
    case EndPoint:   m_pivotPoint = pts.last(); break;
    case MidPoint: {
        double cx = 0, cy = 0;
        for (const QPointF& p : pts) { cx += p.x(); cy += p.y(); }
        m_pivotPoint = QPointF(cx / pts.size(), cy / pts.size());
        break;
    }}
    double rad = qDegreesToRadians(m_degrees);
    double cosA = qCos(rad), sinA = qSin(rad);
    QVector<QPointF> newPts; newPts.reserve(pts.size());
    for (const QPointF& p : pts) {
        double dx = p.x() - m_pivotPoint.x(), dy = p.y() - m_pivotPoint.y();
        newPts.append(QPointF(m_pivotPoint.x() + dx * cosA - dy * sinA,
                              m_pivotPoint.y() + dx * sinA + dy * cosA));
    }
    m_obj->setControlPoints(newPts);
    return true;
}
bool RotateObjectCommand::undo(Project* project) {
    Q_UNUSED(project);
    if (!m_obj) return false;
    m_obj->setControlPoints(m_oldPoints);
    return true;
}

// ============================================================================
// TranslateObjectCommand
// ============================================================================
TranslateObjectCommand::TranslateObjectCommand(CustomObject* obj, const QPointF& delta)
    : Command(QStringLiteral("Translate ") + (obj ? obj->name() : QString()))
    , m_obj(obj), m_delta(delta) {}

bool TranslateObjectCommand::execute(Project* project) {
    Q_UNUSED(project);
    if (!m_obj) return false;
    const auto& pts = m_obj->controlPoints();
    QVector<QPointF> newPts; newPts.reserve(pts.size());
    for (const QPointF& p : pts) newPts.append(p + m_delta);
    m_obj->setControlPoints(newPts);
    return true;
}
bool TranslateObjectCommand::undo(Project* project) {
    Q_UNUSED(project);
    if (!m_obj) return false;
    const auto& pts = m_obj->controlPoints();
    QVector<QPointF> newPts; newPts.reserve(pts.size());
    for (const QPointF& p : pts) newPts.append(p - m_delta);
    m_obj->setControlPoints(newPts);
    return true;
}

// ============================================================================
// CommandHistory
// ============================================================================
CommandHistory::CommandHistory(QObject* parent) : QObject(parent) {}
CommandHistory::~CommandHistory() { clear(); }

bool CommandHistory::push(std::unique_ptr<Command> cmd, Project* project) {
    if (!cmd) return false;
    if (!cmd->execute(project)) return false;

    m_redoStack.clear();
    m_undoStack.push_back(std::move(cmd));

    while (m_undoStack.size() > static_cast<size_t>(kMaxStack))
        m_undoStack.erase(m_undoStack.begin());

    emit stackChanged();
    return true;
}

bool CommandHistory::undo(Project* project) {
    if (m_undoStack.empty()) return false;
    auto cmd = std::move(m_undoStack.back());
    m_undoStack.pop_back();
    if (!cmd->undo(project)) {
        cmd->execute(project);
        m_undoStack.push_back(std::move(cmd));
        return false;
    }
    m_redoStack.push_back(std::move(cmd));
    emit stackChanged();
    return true;
}

bool CommandHistory::redo(Project* project) {
    if (m_redoStack.empty()) return false;
    auto cmd = std::move(m_redoStack.back());
    m_redoStack.pop_back();
    if (!cmd->execute(project)) {
        m_redoStack.push_back(std::move(cmd));
        return false;
    }
    m_undoStack.push_back(std::move(cmd));
    emit stackChanged();
    return true;
}

bool CommandHistory::canUndo() const { return !m_undoStack.empty(); }
bool CommandHistory::canRedo() const { return !m_redoStack.empty(); }

QString CommandHistory::undoText() const {
    if (m_undoStack.empty()) return {};
    return tr("Undo %1").arg(m_undoStack.back()->description());
}
QString CommandHistory::redoText() const {
    if (m_redoStack.empty()) return {};
    return tr("Redo %1").arg(m_redoStack.back()->description());
}
void CommandHistory::clear() {
    m_undoStack.clear();
    m_redoStack.clear();
    emit stackChanged();
}

} // namespace ExpressDesigner