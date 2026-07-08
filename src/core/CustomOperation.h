#pragma once

#include "BaseObject.h"
#include "ObjectTypes.h"
#include <QStringList>
#include <QVector>

namespace ExpressDesigner {

class Project;

class CustomOperation : public BaseObject {
    Q_OBJECT

public:
    explicit CustomOperation(OperationType opType, const QString& name = QString(),
                             QObject* parent = nullptr);
    ~CustomOperation() override;

    OperationType operationType() const;

    // Parameters
    void setAmountOfPoints(int points);
    int amountOfPoints() const;

    const QStringList& paramNames() const;
    void addParamName(const QString& name);
    int paramCount() const;
    QString paramName(int index) const;
    void setParamName(int index, const QString& name);
    bool isNameInParams(const QString& name) const;

    // Execution
    virtual bool execute(Project* project);
    virtual bool isParamObject(int index) const;
    virtual QString resultName() const = 0;
    virtual QString paramPrefixOnTree(int index) const;

    // --- Extra properties (replaces dynamic_cast<PropagateWFOperation*> in commands) ---
    /// Returns names of extra properties (e.g. "offset") for this operation type.
    /// Used by ModifyOperationCommand for generic save/restore.
    virtual QStringList extraPropertyNames() const;

    /// Get the value of an extra property by name. Returns invalid QVariant if not found.
    virtual QVariant extraProperty(const QString& name) const;

    /// Set the value of an extra property by name.
    virtual void setExtraProperty(const QString& name, const QVariant& value);

    // Error
    int errorCode() const;
    QString errorMessage() const;

    // Serialization
    void saveToJson(QJsonObject& json) const override;
    void loadFromJson(const QJsonObject& json) override;

signals:
    void operationExecuted(bool success);

protected:
    OperationType m_opType;
    int m_amountOfPoints = 100;
    QStringList m_paramNames;
    int m_errorCode = 0;
    QString m_errorMessage;
};

} // namespace ExpressDesigner