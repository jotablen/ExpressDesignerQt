#pragma once
#include <QObject>
#include <QVector>
#include <QPointF>
#include <QString>
#include <QDateTime>
#include <memory>

namespace ExpressDesigner {

class Project;
class CustomObject;
class CustomOperation;

// ============================================================================
// Command — Abstract base
// ============================================================================
class Command {
public:
    explicit Command(const QString& description = {});
    virtual ~Command();
    virtual bool execute(Project* project) = 0;
    virtual bool undo(Project* project) = 0;
    QString description() const;
    QDateTime timestamp() const;

    /// Returns the name of the object that was modified by this command.
    /// Used by CommandHistory for targeted recalculation after undo/redo.
    /// Empty string means "unknown/recalc everything".
    virtual QString modifiedObjectName() const { return {}; }
protected:
    QString m_description;
    QDateTime m_timestamp;
};

// ============================================================================
// Concrete commands
// ============================================================================
class AddObjectCommand : public Command {
public:
    explicit AddObjectCommand(CustomObject* obj);
    bool execute(Project* project) override;
    bool undo(Project* project) override;
    CustomObject* object() const;
private:
    CustomObject* m_obj;
    bool m_wasResult = false;
};

class DeleteObjectCommand : public Command {
public:
    explicit DeleteObjectCommand(CustomObject* obj, bool isResult);
    bool execute(Project* project) override;
    bool undo(Project* project) override;
private:
    CustomObject* m_obj;
    bool m_isResult;
    int m_index = -1;
};

class ModifyObjectCommand : public Command {
public:
    ModifyObjectCommand(CustomObject* obj, const QString& property,
                       const QVariant& oldVal, const QVariant& newVal);
    bool execute(Project* project) override;
    bool undo(Project* project) override;
    QString modifiedObjectName() const override;
private:
    CustomObject* m_obj;
    QString m_property;
    QVariant m_oldValue;
    QVariant m_newValue;
};

class ModifyObjectPropertiesCommand : public Command {
public:
    ModifyObjectPropertiesCommand(CustomObject* obj,
        const QString& oldName, double oldIR, bool oldFlip, const QVector<QPointF>& oldPts,
        const QString& newName, double newIR, bool newFlip, const QVector<QPointF>& newPts);
    bool execute(Project* project) override;
    bool undo(Project* project) override;
    QString modifiedObjectName() const override;
private:
    CustomObject* m_obj;
    QString m_oldName, m_newName;
    double m_oldIR, m_newIR;
    bool m_oldFlip, m_newFlip;
    QVector<QPointF> m_oldPts, m_newPts;
};

class ModifyControlPointsCommand : public Command {
public:
    ModifyControlPointsCommand(CustomObject* obj,
                               const QVector<QPointF>& oldPts,
                               const QVector<QPointF>& newPts);
    bool execute(Project* project) override;
    bool undo(Project* project) override;
    QString modifiedObjectName() const override;
private:
    CustomObject* m_obj;
    QVector<QPointF> m_oldPoints;
    QVector<QPointF> m_newPoints;
};

class ExecuteOperationCommand : public Command {
public:
    explicit ExecuteOperationCommand(CustomOperation* op);
    bool execute(Project* project) override;
    bool undo(Project* project) override;
    CustomOperation* operation() const;
    CustomObject* resultObject() const;
    QString modifiedObjectName() const override;
private:
    CustomOperation* m_op;
    CustomObject* m_resultObj = nullptr;
    bool m_wasAdded = false;
};

class RotateObjectCommand : public Command {
public:
    enum PivotMode { StartPoint, MidPoint, EndPoint };
    RotateObjectCommand(CustomObject* obj, double degrees, PivotMode pivot);
    bool execute(Project* project) override;
    bool undo(Project* project) override;
    CustomObject* object() const { return m_obj; }
    QString modifiedObjectName() const override;
private:
    CustomObject* m_obj;
    double m_degrees;
    PivotMode m_pivot;
    QVector<QPointF> m_oldPoints;
    QPointF m_pivotPoint;
};

class TranslateObjectCommand : public Command {
public:
    TranslateObjectCommand(CustomObject* obj, const QPointF& delta);
    bool execute(Project* project) override;
    bool undo(Project* project) override;
    CustomObject* object() const { return m_obj; }
    QString modifiedObjectName() const override;
private:
    CustomObject* m_obj;
    QPointF m_delta;
};

class ModifyOperationCommand : public Command {
public:
    ModifyOperationCommand(CustomOperation* op,
        const QString& oldName, int oldQty, const QStringList& oldParams,
        const QString& oldResultName, double oldOffset,
        const QString& newName, int newQty, const QStringList& newParams,
        const QString& newResultName, double newOffset);
    bool execute(Project* project) override;
    bool undo(Project* project) override;
    QString modifiedObjectName() const override;
private:
    CustomOperation* m_op;
    QString m_oldName, m_newName;
    int m_oldQty, m_newQty;
    QStringList m_oldParams, m_newParams;
    QString m_oldResultName, m_newResultName;
    double m_oldOffset, m_newOffset;
};

// ============================================================================
// CommandHistory — Stack-based undo/redo
// ============================================================================
class CommandHistory : public QObject {
    Q_OBJECT
public:
    explicit CommandHistory(QObject* parent = nullptr);
    ~CommandHistory() override;

    bool push(std::unique_ptr<Command> cmd, Project* project);
    bool undo(Project* project);
    bool redo(Project* project);

    bool canUndo() const;
    bool canRedo() const;
    QString undoText() const;
    QString redoText() const;
    void clear();

    /// Returns the modifiedObjectName() of the command that was just undone.
    /// Only valid immediately after undo() returns true.
    QString lastUndoneModifiedObjectName() const;

    /// Returns the modifiedObjectName() of the command that was just redone.
    /// Only valid immediately after redo() returns true.
    QString lastRedoneModifiedObjectName() const;

signals:
    void stackChanged();

private:
    static constexpr int kMaxStack = 200;
    std::vector<std::unique_ptr<Command>> m_undoStack;
    std::vector<std::unique_ptr<Command>> m_redoStack;
    QString m_lastUndoneObjName;
    QString m_lastRedoneObjName;
};

} // namespace ExpressDesigner