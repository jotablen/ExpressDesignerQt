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
    : m_description(description)
    , m_timestamp(QDateTime::currentDateTime())
{}

Command::~Command() = default;

QString Command::description() const { return m_description; }
QDateTime Command::timestamp() const { return m_timestamp; }

// ============================================================================
// AddObjectCommand
// ============================================================================
AddObjectCommand::AddObjectCommand(CustomObject* obj)
    : Command(QStringLiteral("Add ") + (obj ? obj->name() : QString()))
    , m_obj(obj)
    , m_wasResult(false)
{}

bool AddObjectCommand::execute(Project* project)
{
    if (!project || !m_obj) return false;
    if (m_obj->isResult())
        project->addResultObject(m_obj);
    else
        project->addDataObject(m_obj);
    return true;
}

bool AddObjectCommand::undo(Project* project)
{
    if (!project || !m_obj) return false;
    m_wasResult = m_obj->isResult();
    if (m_wasResult)
        project->removeResultObject(m_obj);
    else
        project->removeDataObject(m_obj);
    return true;
}

CustomObject* AddObjectCommand::object() const { return m_obj; }

// ============================================================================
// DeleteObjectCommand
// ============================================================================
DeleteObjectCommand::DeleteObjectCommand(CustomObject* obj, bool isResult)
    : Command(QStringLiteral("Delete ") + (obj ? obj->name() : QString()))
    , m_obj(obj)
    , m_isResult(isResult)
    , m_index(-1)
{}

bool DeleteObjectCommand::execute(Project* project)
{
    if (!project || !m_obj) return false;
    if (m_isResult) {
        m_index = project->findObjectIndex(m_obj);
        project->removeResultObject(m_obj);
    } else {
        m_index = project->findObjectIndex(m_obj);
        project->removeDataObject(m_obj);
    }
    return true;
}

bool DeleteObjectCommand::undo(Project* project)
{
    if (!project || !m_obj) return false;
    if (m_isResult)
        project->addResultObject(m_obj);
    else
        project->addDataObject(m_obj);
    return true;
}

CustomObject* DeleteObjectCommand::object() const { return m_obj; }

// ============================================================================
// ModifyObjectCommand
// ============================================================================
ModifyObjectCommand::ModifyObjectCommand(CustomObject* obj, const QString& propertyName,
                                         const QVariant& oldValue, const QVariant& newValue)
    : Command(QStringLiteral("Modify %1").arg(obj ? obj->name() : QString()))
    , m_obj(obj)
    , m_propertyName(propertyName)
    , m_oldValue(oldValue)
    , m_newValue(newValue)
{}

bool ModifyObjectCommand::execute(Project* project)
{
    Q_UNUSED(project);
    if (!m_obj) return false;
    // Apply new value based on property name
    if (m_propertyName == QStringLiteral("name"))
        m_obj->setName(m_newValue.toString());
    else if (m_propertyName == QStringLiteral("refractiveIndex"))
        m_obj->setRefractiveIndex(m_newValue.toDouble());
    else if (m_propertyName == QStringLiteral("visible"))
        m_obj->setVisible(m_newValue.toBool());
    else if (m_propertyName == QStringLiteral("labelVisible"))
        m_obj->setLabelVisible(m_newValue.toBool());
    else if (m_propertyName == QStringLiteral("normalFlipped"))
        m_obj->setNormalFlipped(m_newValue.toBool());
    else if (m_propertyName == QStringLiteral("referencePoint"))
        m_obj->setReferencePoint(m_newValue.toPointF());
    // More properties can be added as needed
    return true;
}

bool ModifyObjectCommand::undo(Project* project)
{
    Q_UNUSED(project);
    if (!m_obj) return false;
    // Restore old value
    if (m_propertyName == QStringLiteral("name"))
        m_obj->setName(m_oldValue.toString());
    else if (m_propertyName == QStringLiteral("refractiveIndex"))
        m_obj->setRefractiveIndex(m_oldValue.toDouble());
    else if (m_propertyName == QStringLiteral("visible"))
        m_obj->setVisible(m_oldValue.toBool());
    else if (m_propertyName == QStringLiteral("labelVisible"))
        m_obj->setLabelVisible(m_oldValue.toBool());
    else if (m_propertyName == QStringLiteral("normalFlipped"))
        m_obj->setNormalFlipped(m_oldValue.toBool());
    else if (m_propertyName == QStringLiteral("referencePoint"))
        m_obj->setReferencePoint(m_oldValue.toPointF());
    return true;
}

// ============================================================================
// ModifyControlPointsCommand
// ============================================================================
ModifyControlPointsCommand::ModifyControlPointsCommand(CustomObject* obj,
                                                       const QVector<QPointF>& oldPts,
                                                       const QVector<QPointF>& newPts)
    : Command(QStringLiteral("Edit points of ") + (obj ? obj->name() : QString()))
    , m_obj(obj)
    , m_oldPoints(oldPts)
    , m_newPoints(newPts)
{}

bool ModifyControlPointsCommand::execute(Project* project)
{
    Q_UNUSED(project);
    if (!m_obj) return false;
    m_obj->setControlPoints(m_newPoints);
    return true;
}

bool ModifyControlPointsCommand::undo(Project* project)
{
    Q_UNUSED(project);
    if (!m_obj) return false;
    m_obj->setControlPoints(m_oldPoints);
    return true;
}

// ============================================================================
// ExecuteOperationCommand
// ============================================================================
ExecuteOperationCommand::ExecuteOperationCommand(CustomOperation* op)
    : Command(QStringLiteral("Execute ") + (op ? op->name() : QString()))
    , m_op(op)
    , m_resultObj(nullptr)
    , m_wasAdded(false)
{}

bool ExecuteOperationCommand::execute(Project* project)
{
    if (!project || !m_op) return false;

    // Find previous result if this op was already executed
    QString resultName = m_op->resultName();
    m_resultObj = project->findObject(resultName);

    if (m_resultObj) {
        // Remove old object from BOTH lists before re-executing (avoid rename on recalc)
        project->removeResultObject(m_resultObj);
        project->removeDataObject(m_resultObj);
        if (m_op->execute(project)) {
            m_resultObj = project->findObject(resultName);
        } else {
            m_resultObj = nullptr;
        }
        m_wasAdded = false;
    } else {
        // First execution
        if (m_op->execute(project)) {
            m_resultObj = project->findObject(resultName);
            m_wasAdded = true;
        }
    }
    return true;
}

bool ExecuteOperationCommand::undo(Project* project)
{
    if (!project || !m_resultObj) return false;
    if (m_wasAdded) {
        project->removeResultObject(m_resultObj);
        project->removeOperation(m_op);
    }
    return true;
}

CustomOperation* ExecuteOperationCommand::operation() const { return m_op; }
CustomObject* ExecuteOperationCommand::resultObject() const { return m_resultObj; }

// ============================================================================
// RotateObjectCommand
// ============================================================================
RotateObjectCommand::RotateObjectCommand(CustomObject* obj, double degrees, PivotMode pivot)
    : Command(QStringLiteral("Rotate ") + (obj ? obj->name() : QString()))
    , m_obj(obj)
    , m_degrees(degrees)
    , m_pivot(pivot)
{}

bool RotateObjectCommand::execute(Project* project)
{
    Q_UNUSED(project);
    if (!m_obj) return false;

    const QVector<QPointF>& pts = m_obj->controlPoints();
    if (pts.isEmpty()) return false;
    m_oldPoints = pts;

    // Determine pivot point
    switch (m_pivot) {
    case StartPoint:
        m_pivotPoint = pts.first();
        break;
    case EndPoint:
        m_pivotPoint = pts.last();
        break;
    case MidPoint: {
        double cx = 0.0, cy = 0.0;
        for (const QPointF& p : pts) { cx += p.x(); cy += p.y(); }
        m_pivotPoint = QPointF(cx / pts.size(), cy / pts.size());
        break;
    }
    }

    // Rotate
    double rad = qDegreesToRadians(m_degrees);
    double cosA = qCos(rad);
    double sinA = qSin(rad);
    QVector<QPointF> newPts;
    newPts.reserve(pts.size());

    for (const QPointF& p : pts) {
        double dx = p.x() - m_pivotPoint.x();
        double dy = p.y() - m_pivotPoint.y();
        double rx = dx * cosA - dy * sinA;
        double ry = dx * sinA + dy * cosA;
        newPts.append(QPointF(m_pivotPoint.x() + rx, m_pivotPoint.y() + ry));
    }

    m_obj->setControlPoints(newPts);
    return true;
}

bool RotateObjectCommand::undo(Project* project)
{
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
    , m_obj(obj)
    , m_delta(delta)
{}

bool TranslateObjectCommand::execute(Project* project)
{
    Q_UNUSED(project);
    if (!m_obj) return false;

    const QVector<QPointF>& pts = m_obj->controlPoints();
    QVector<QPointF> newPts;
    newPts.reserve(pts.size());
    for (const QPointF& p : pts)
        newPts.append(p + m_delta);

    m_obj->setControlPoints(newPts);
    return true;
}

bool TranslateObjectCommand::undo(Project* project)
{
    Q_UNUSED(project);
    if (!m_obj) return false;

    const QVector<QPointF>& pts = m_obj->controlPoints();
    QVector<QPointF> newPts;
    newPts.reserve(pts.size());
    for (const QPointF& p : pts)
        newPts.append(p - m_delta);

    m_obj->setControlPoints(newPts);
    return true;
}

// ============================================================================
// CompoundCommand
// ============================================================================
CompoundCommand::CompoundCommand(const QString& description)
    : Command(description)
{}

CompoundCommand::~CompoundCommand()
{
    qDeleteAll(m_commands);
    m_commands.clear();
}

void CompoundCommand::addCommand(Command* cmd)
{
    if (cmd) m_commands.append(cmd);
}

bool CompoundCommand::execute(Project* project)
{
    for (auto* cmd : m_commands) {
        if (!cmd->execute(project))
            return false;
    }
    return true;
}

bool CompoundCommand::undo(Project* project)
{
    // Undo in reverse order
    for (int i = m_commands.size() - 1; i >= 0; --i) {
        if (!m_commands[i]->undo(project))
            return false;
    }
    return true;
}

int CompoundCommand::count() const { return m_commands.size(); }

// ============================================================================
// CommandHistory
// ============================================================================
CommandHistory::CommandHistory(QObject* parent) : QObject(parent) {}

CommandHistory::~CommandHistory()
{
    clear();
}

bool CommandHistory::push(Command* cmd, Project* project)
{
    if (!cmd) return false;
    if (!cmd->execute(project)) {
        delete cmd;
        return false;
    }

    qDeleteAll(m_redoStack);
    m_redoStack.clear();

    m_undoStack.append(cmd);

    // Limit stack size to avoid memory issues
    const int maxStack = 200;
    while (m_undoStack.size() > maxStack) {
        delete m_undoStack.takeFirst();
    }

    emit stackChanged();
    return true;
}

bool CommandHistory::undo(Project* project)
{
    if (m_undoStack.isEmpty()) return false;

    Command* cmd = m_undoStack.takeLast();
    if (cmd->undo(project)) {
        m_redoStack.append(cmd);
        emit stackChanged();
        return true;
    }

    // Undo failed — try to redo to restore state
    cmd->execute(project);
    m_undoStack.append(cmd);
    return false;
}

bool CommandHistory::redo(Project* project)
{
    if (m_redoStack.isEmpty()) return false;

    Command* cmd = m_redoStack.takeLast();
    if (cmd->execute(project)) {
        m_undoStack.append(cmd);
        emit stackChanged();
        return true;
    }

    m_redoStack.append(cmd);
    return false;
}

bool CommandHistory::canUndo() const { return !m_undoStack.isEmpty(); }
bool CommandHistory::canRedo() const { return !m_redoStack.isEmpty(); }

QString CommandHistory::undoText() const
{
    if (m_undoStack.isEmpty()) return QString();
    return tr("Undo %1").arg(m_undoStack.last()->description());
}

QString CommandHistory::redoText() const
{
    if (m_redoStack.isEmpty()) return QString();
    return tr("Redo %1").arg(m_redoStack.last()->description());
}

void CommandHistory::clear()
{
    qDeleteAll(m_undoStack);
    qDeleteAll(m_redoStack);
    m_undoStack.clear();
    m_redoStack.clear();
    emit stackChanged();
}

int CommandHistory::undoCount() const { return m_undoStack.size(); }
int CommandHistory::redoCount() const { return m_redoStack.size(); }

} // namespace ExpressDesigner