#pragma once

#include <QObject>
#include <QVector>
#include <QPointF>
#include <QString>
#include <QDateTime>

namespace ExpressDesigner {

class Project;
class CustomObject;
class CustomOperation;

// ============================================================================
// Command — Abstract base for undoable actions
// ============================================================================
class Command {
public:
    explicit Command(const QString& description = QString());
    virtual ~Command();

    virtual bool execute(Project* project) = 0;
    virtual bool undo(Project* project) = 0;

    QString description() const;
    QDateTime timestamp() const;

protected:
    QString m_description;
    QDateTime m_timestamp;
};

// ============================================================================
// AddObjectCommand
// ============================================================================
class AddObjectCommand : public Command {
public:
    explicit AddObjectCommand(CustomObject* obj);
    bool execute(Project* project) override;
    bool undo(Project* project) override;
    CustomObject* object() const;
private:
    CustomObject* m_obj;
    bool m_wasResult;
};

// ============================================================================
// DeleteObjectCommand
// ============================================================================
class DeleteObjectCommand : public Command {
public:
    explicit DeleteObjectCommand(CustomObject* obj, bool isResult);
    bool execute(Project* project) override;
    bool undo(Project* project) override;
    CustomObject* object() const;
private:
    CustomObject* m_obj;
    bool m_isResult;
    int m_index;
};

// ============================================================================
// ModifyObjectCommand
// ============================================================================
class ModifyObjectCommand : public Command {
public:
    ModifyObjectCommand(CustomObject* obj, const QString& propertyName,
                        const QVariant& oldValue, const QVariant& newValue);
    bool execute(Project* project) override;
    bool undo(Project* project) override;
private:
    CustomObject* m_obj;
    QString m_propertyName;
    QVariant m_oldValue;
    QVariant m_newValue;
};

// ============================================================================
// ModifyControlPointsCommand
// ============================================================================
class ModifyControlPointsCommand : public Command {
public:
    ModifyControlPointsCommand(CustomObject* obj,
                               const QVector<QPointF>& oldPts,
                               const QVector<QPointF>& newPts);
    bool execute(Project* project) override;
    bool undo(Project* project) override;
private:
    CustomObject* m_obj;
    QVector<QPointF> m_oldPoints;
    QVector<QPointF> m_newPoints;
};

// ============================================================================
// ExecuteOperationCommand
// ============================================================================
class ExecuteOperationCommand : public Command {
public:
    ExecuteOperationCommand(CustomOperation* op);
    bool execute(Project* project) override;
    bool undo(Project* project) override;
    CustomOperation* operation() const;
    CustomObject* resultObject() const;
private:
    CustomOperation* m_op;
    CustomObject* m_resultObj;
    bool m_wasAdded;
};

// ============================================================================
// RotateObjectCommand
// ============================================================================
class RotateObjectCommand : public Command {
public:
    enum PivotMode { StartPoint, MidPoint, EndPoint };

    RotateObjectCommand(CustomObject* obj, double degrees, PivotMode pivot);
    bool execute(Project* project) override;
    bool undo(Project* project) override;
private:
    CustomObject* m_obj;
    double m_degrees;
    PivotMode m_pivot;
    QVector<QPointF> m_oldPoints;
    QPointF m_pivotPoint;
};

// ============================================================================
// TranslateObjectCommand
// ============================================================================
class TranslateObjectCommand : public Command {
public:
    TranslateObjectCommand(CustomObject* obj, const QPointF& delta);
    bool execute(Project* project) override;
    bool undo(Project* project) override;
private:
    CustomObject* m_obj;
    QPointF m_delta;
};

// ============================================================================
// CompoundCommand — Group multiple commands into one undoable unit
// ============================================================================
class CompoundCommand : public Command {
public:
    explicit CompoundCommand(const QString& description = QString());
    ~CompoundCommand() override;

    void addCommand(Command* cmd);
    bool execute(Project* project) override;
    bool undo(Project* project) override;
    int count() const;

private:
    QVector<Command*> m_commands;
};

// ============================================================================
// CommandHistory — Stack-based undo/redo manager
// ============================================================================
class CommandHistory : public QObject {
    Q_OBJECT

public:
    explicit CommandHistory(QObject* parent = nullptr);
    ~CommandHistory() override;

    /** Execute a command and push it onto the undo stack.
     *  Clears the redo stack. Returns true on success. */
    bool push(Command* cmd, Project* project);

    /** Undo the last command. Returns true on success. */
    bool undo(Project* project);

    /** Redo the last undone command. Returns true on success. */
    bool redo(Project* project);

    bool canUndo() const;
    bool canRedo() const;
    QString undoText() const;
    QString redoText() const;

    void clear();
    int undoCount() const;
    int redoCount() const;

    const QVector<Command*>& undoStack() const { return m_undoStack; }
    const QVector<Command*>& redoStack() const { return m_redoStack; }

signals:
    void stackChanged();

private:
    QVector<Command*> m_undoStack;
    QVector<Command*> m_redoStack;
};

} // namespace ExpressDesigner