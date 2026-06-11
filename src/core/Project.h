#pragma once

#include "BaseObject.h"
#include "CustomObject.h"
#include "CustomOperation.h"
#include <QVector>
#include <functional>

namespace ExpressDesigner {

class Project : public BaseObject {
    Q_OBJECT

public:
    explicit Project(const QString& name = QStringLiteral("Unnamed Project"),
                     QObject* parent = nullptr);
    ~Project() override;

    // Object lists
    const QVector<CustomObject*>& dataObjects() const;
    const QVector<CustomObject*>& resultObjects() const;
    QVector<CustomObject*> allObjects() const;

    int dataObjectCount() const;
    int resultObjectCount() const;

    // Add/Remove objects
    void addDataObject(CustomObject* obj);
    void insertDataObject(int index, CustomObject* obj);
    void removeDataObject(int index);
    void removeDataObject(CustomObject* obj);
    void moveDataObjectToResult(int index);

    void addResultObject(CustomObject* obj);
    void removeResultObject(int index);
    void removeResultObject(CustomObject* obj);

    // Find objects
    CustomObject* findObject(const QUuid& uuid) const;
    CustomObject* findObject(const QString& name) const;
    int findObjectIndex(const CustomObject* obj) const;

    bool isNameInUse(const QString& name) const;
    bool isUUIDInUse(const QUuid& uuid) const;

    // Operations
    const QVector<CustomOperation*>& operations() const;
    void addOperation(CustomOperation* op);
    void removeOperation(int index);
    void removeOperation(CustomOperation* op);
    bool isOperationNameInUse(const QString& name) const;

    // Scale
    void setScaleX(double min, double max);
    double scaleXMin() const;
    double scaleXMax() const;
    void setScaleY(double min, double max);
    double scaleYMin() const;
    double scaleYMax() const;
    void setAutoScaleX(bool autoScale);
    bool autoScaleX() const;
    void setAutoScaleY(bool autoScale);
    bool autoScaleY() const;

    // Aspect ratio
    enum class AspectRatioMode { Maintain, FixedX, FixedY };
    void setAspectRatioMode(AspectRatioMode mode);
    AspectRatioMode aspectRatioMode() const;

    // Clear
    void clearAll();

    // Serialization
    void saveToJson(QJsonObject& json) const override;
    void loadFromJson(const QJsonObject& json) override;

signals:
    void dataObjectAdded(CustomObject* obj, int index);
    void dataObjectRemoved(CustomObject* obj, int index);
    void resultObjectAdded(CustomObject* obj, int index);
    void resultObjectRemoved(CustomObject* obj, int index);
    void operationAdded(CustomOperation* op, int index);
    void operationRemoved(CustomOperation* op, int index);
    void scaleChanged();
    void projectCleared();

private:
    QVector<CustomObject*> m_dataObjects;
    QVector<CustomObject*> m_resultObjects;
    QVector<CustomOperation*> m_operations;

    double m_scaleXMin = -10.0;
    double m_scaleXMax = 10.0;
    double m_scaleYMin = -10.0;
    double m_scaleYMax = 10.0;
    bool m_autoScaleX = true;
    bool m_autoScaleY = true;
    AspectRatioMode m_aspectRatioMode = AspectRatioMode::Maintain;

    void clearObjectList(QVector<CustomObject*>& list);
    void clearOperationList(QVector<CustomOperation*>& list);
};

} // namespace ExpressDesigner