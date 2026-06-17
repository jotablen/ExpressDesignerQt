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
    virtual QString modifiedObjectName() const { return {}; }
    QString description() const;
    QDateTime timestamp() const;
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
private:
    CustomObject* m_obj;
    QString m_property;
    QVariant m_oldValue;
    QVariant m_newValue;
};

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

class ExecuteOperationCommand : public Command {
public:
    explicit ExecuteOperationCommand(CustomOperation* op);
    bool execute(Project* project) override;
    bool undo(Project* project) override;
    CustomOperation* operation() const;
    CustomObject* resultObject() const;
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
    QString lastUndoneModifiedObjectName() const;
    QString lastRedoneModifiedObjectName() const;
    void clear();

signals:
    void stackChanged();

private:
    static constexpr int kMaxStack = 200;
    std::vector<std::unique_ptr<Command>> m_undoStack;
    std::vector<std::unique_ptr<Command>> m_redoStack;
};

} // namespace ExpressDesigner