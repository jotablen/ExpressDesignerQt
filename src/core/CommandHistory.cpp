#include "CommandHistory.h"
#include "CustomObject.h"
#include "CustomOperation.h"
#include "Project.h"
#include "ArcObject.h"
#include <QtMath>
#include <QVariant>
#include <utils/Logger.h>

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
// ModifyObjectPropertiesCommand — saves ALL properties at once for undo
// ============================================================================
ModifyObjectPropertiesCommand::ModifyObjectPropertiesCommand(CustomObject* obj,
    const QString& oldName, double oldIR, bool oldFlip, const QVector<QPointF>& oldPts,
    const QString& newName, double newIR, bool newFlip, const QVector<QPointF>& newPts)
    : Command(QStringLiteral("Modify ") + (obj ? obj->name() : QString()))
    , m_obj(obj), m_oldName(oldName), m_newName(newName)
    , m_oldIR(oldIR), m_newIR(newIR)
    , m_oldFlip(oldFlip), m_newFlip(newFlip)
    , m_oldPts(oldPts), m_newPts(newPts)
{
}

bool ModifyObjectPropertiesCommand::execute(Project* project) {
    Q_UNUSED(project);
    if (!m_obj) return false;
    m_obj->setName(m_newName);
    m_obj->setRefractiveIndex(m_newIR);
    m_obj->setNormalFlipped(m_newFlip);
    if (!m_newPts.isEmpty()) m_obj->setControlPoints(m_newPts);
    return true;
}
bool ModifyObjectPropertiesCommand::undo(Project* project) {
    Q_UNUSED(project);
    if (!m_obj) return false;
    m_obj->setName(m_oldName);
    m_obj->setRefractiveIndex(m_oldIR);
    m_obj->setNormalFlipped(m_oldFlip);
    if (!m_oldPts.isEmpty()) m_obj->setControlPoints(m_oldPts);
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
    TRACE_CAT(QStringLiteral("CMD"));
    if (!project || !m_op) { LOG_WARN(QStringLiteral("CMD"), QStringLiteral("ExecuteCMD: null project or op")); return false; }
    QString resultName = m_op->resultName();
    LOG_INFO(QStringLiteral("CMD"), QStringLiteral("ExecuteCMD: execute() name='%1'").arg(resultName));

    if (!project->operations().contains(m_op)) {
        LOG_INFO(QStringLiteral("CMD"), QStringLiteral("ExecuteCMD: re-adding op '%1' to project").arg(m_op->name()));
        project->addOperation(m_op);
    }

    LOG_INFO(QStringLiteral("CMD"), QStringLiteral("ExecuteCMD: calling op->execute()..."));
    int prevResultCount = project->resultObjectCount();
    if (!m_op->execute(project)) {
        LOG_ERROR(QStringLiteral("CMD"), QStringLiteral("ExecuteCMD: op->execute() FAILED — removing operation"));
        project->takeOperation(m_op);
        return false;
    }

    if (project->resultObjectCount() > prevResultCount)
        m_resultObj = project->resultObjects().last();
    else
        m_resultObj = project->findObject(resultName);

    m_wasAdded = true;
    LOG_INFO(QStringLiteral("CMD"), QStringLiteral("ExecuteCMD: OK, result=%1").arg(m_resultObj ? m_resultObj->name() : QStringLiteral("NULL")));
    return true;
}
bool ExecuteOperationCommand::undo(Project* project) {
    TRACE_CAT(QStringLiteral("CMD"));
    if (!project || !m_resultObj || !m_wasAdded) { LOG_WARN(QStringLiteral("CMD"), QStringLiteral("ExecuteCMD: undo() precondition failed")); return false; }
    LOG_INFO(QStringLiteral("CMD"), QStringLiteral("ExecuteCMD: undo() removing result '%1'").arg(m_resultObj->name()));
    project->removeResultObject(m_resultObj);
    m_resultObj = nullptr;
    m_wasAdded = false;
    LOG_INFO(QStringLiteral("CMD"), QStringLiteral("ExecuteCMD: undo() taking op '%1' from project").arg(m_op->name()));
    project->takeOperation(m_op);
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
    // Also translate Arc center point if applicable
    if (auto* arc = qobject_cast<ArcObject*>(m_obj)) {
        arc->setCenter(arc->center() + m_delta);
    }
    return true;
}

bool TranslateObjectCommand::undo(Project* project) {
    Q_UNUSED(project);
    if (!m_obj) return false;
    const auto& pts = m_obj->controlPoints();
    QVector<QPointF> newPts; newPts.reserve(pts.size());
    for (const QPointF& p : pts) newPts.append(p - m_delta);
    m_obj->setControlPoints(newPts);
    if (auto* arc = qobject_cast<ArcObject*>(m_obj)) {
        arc->setCenter(arc->center() - m_delta);
    }
    return true;
}

// ============================================================================
// CommandHistory
// ============================================================================
CommandHistory::CommandHistory(QObject* parent) : QObject(parent) {}
CommandHistory::~CommandHistory() { clear(); }

bool CommandHistory::push(std::unique_ptr<Command> cmd, Project* project) {
    TRACE_CAT(QStringLiteral("CMD"));
    if (!cmd) { LOG_WARN(QStringLiteral("CMD"), QStringLiteral("push: null command")); return false; }
    LOG_INFO(QStringLiteral("CMD"), QStringLiteral("push: %1 (undoStack=%2)").arg(cmd->description()).arg(m_undoStack.size() + 1));
    if (!cmd->execute(project)) { LOG_WARN(QStringLiteral("CMD"), QStringLiteral("push: execute failed for '%1'").arg(cmd->description())); return false; }

    m_redoStack.clear();
    m_undoStack.push_back(std::move(cmd));

    while (m_undoStack.size() > static_cast<size_t>(kMaxStack))
        m_undoStack.erase(m_undoStack.begin());

    emit stackChanged();
    return true;
}

bool CommandHistory::undo(Project* project) {
    TRACE_CAT(QStringLiteral("CMD"));
    if (m_undoStack.empty()) return false;
    auto cmd = std::move(m_undoStack.back());
    LOG_INFO(QStringLiteral("CMD"), QStringLiteral("undo: %1 (undoStack=%2)").arg(cmd->description()).arg(m_undoStack.size() - 1));
    m_undoStack.pop_back();
    if (!cmd->undo(project)) {
        LOG_ERROR(QStringLiteral("CMD"), QStringLiteral("undo: FAILED for '%1' — re-executing").arg(cmd->description()));
        cmd->execute(project);
        m_undoStack.push_back(std::move(cmd));
        return false;
    }
    m_redoStack.push_back(std::move(cmd));
    emit stackChanged();
    LOG_INFO(QStringLiteral("CMD"), QStringLiteral("undo: OK (redoStack=%1)").arg(m_redoStack.size()));
    return true;
}

bool CommandHistory::redo(Project* project) {
    TRACE_CAT(QStringLiteral("CMD"));
    if (m_redoStack.empty()) return false;
    auto cmd = std::move(m_redoStack.back());
    LOG_INFO(QStringLiteral("CMD"), QStringLiteral("redo: %1 (redoStack=%2)").arg(cmd->description()).arg(m_redoStack.size() - 1));
    m_redoStack.pop_back();
    if (!cmd->execute(project)) {
        LOG_ERROR(QStringLiteral("CMD"), QStringLiteral("redo: FAILED for '%1'").arg(cmd->description()));
        m_redoStack.push_back(std::move(cmd));
        return false;
    }
    m_undoStack.push_back(std::move(cmd));
    emit stackChanged();
    LOG_INFO(QStringLiteral("CMD"), QStringLiteral("redo: OK (undoStack=%1)").arg(m_undoStack.size()));
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