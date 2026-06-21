#include "Project.h"
#include "ObjectFactory.h"
#include <QJsonArray>
#include <algorithm>

namespace ExpressDesigner {

Project::Project(const QString& name, QObject* parent)
    : BaseObject(ObjectType::Mask_Project, name, parent)
{
}

Project::~Project()
{
    clearAll();
}

const QVector<CustomObject*>& Project::dataObjects() const { return m_dataObjects; }
const QVector<CustomObject*>& Project::resultObjects() const { return m_resultObjects; }

QVector<CustomObject*> Project::allObjects() const
{
    QVector<CustomObject*> result;
    result.reserve(m_dataObjects.size() + m_resultObjects.size());
    result.append(m_dataObjects);
    result.append(m_resultObjects);
    return result;
}

int Project::dataObjectCount() const { return m_dataObjects.size(); }
int Project::resultObjectCount() const { return m_resultObjects.size(); }

void Project::addDataObject(CustomObject* obj)
{
    if (!obj) return;
    obj->setParent(this);
    int idx = m_dataObjects.size();
    m_dataObjects.append(obj);
    emit dataObjectAdded(obj, idx);
    emit objectModified(QStringLiteral("dataObjects"));
}

void Project::insertDataObject(int index, CustomObject* obj)
{
    if (!obj) return;
    obj->setParent(this);
    int idx = qBound(0, index, m_dataObjects.size());
    m_dataObjects.insert(idx, obj);
    emit dataObjectAdded(obj, idx);
    emit objectModified(QStringLiteral("dataObjects"));
}

void Project::removeDataObject(int index)
{
    if (index < 0 || index >= m_dataObjects.size()) return;
    CustomObject* obj = m_dataObjects.takeAt(index);
    emit dataObjectRemoved(obj, index);
    emit objectModified(QStringLiteral("dataObjects"));
    obj->deleteLater();
}

void Project::removeDataObject(CustomObject* obj)
{
    int idx = m_dataObjects.indexOf(obj);
    if (idx >= 0) removeDataObject(idx);
}

void Project::moveDataObjectToResult(int index)
{
    if (index < 0 || index >= m_dataObjects.size()) return;
    CustomObject* obj = m_dataObjects.takeAt(index);
    obj->setObjectType(withResult(obj->objectType()));
    int newIdx = m_resultObjects.size();
    m_resultObjects.append(obj);
    emit dataObjectRemoved(obj, index);
    emit resultObjectAdded(obj, newIdx);
    emit objectModified(QStringLiteral("dataObjects"));
    emit objectModified(QStringLiteral("resultObjects"));
}

void Project::addResultObject(CustomObject* obj)
{
    if (!obj) return;
    obj->setParent(this);

    // Ensure unique name (auto-rename only if another OBJECT has the same name)
    // Operations may share names with their results — that is intentional (ODs behavior)
    CustomObject* existing = findObject(obj->name());
    if (existing && existing != obj) {
        QString baseName = obj->name();
        int suffix = 2;
        QString newName;
        do {
            newName = baseName + QStringLiteral("_") + QString::number(suffix);
            ++suffix;
        } while (findObject(newName) != nullptr);
        QString oldName = baseName;
        obj->setName(newName);
        emit objectAutoRenamed(oldName, newName);
    }

    int idx = m_resultObjects.size();
    m_resultObjects.append(obj);
    emit resultObjectAdded(obj, idx);
    emit objectModified(QStringLiteral("resultObjects"));
}

void Project::removeResultObject(int index)
{
    if (index < 0 || index >= m_resultObjects.size()) return;
    CustomObject* obj = m_resultObjects.takeAt(index);
    emit resultObjectRemoved(obj, index);
    emit objectModified(QStringLiteral("resultObjects"));
    delete obj;  // immediate to avoid stale pointers in dependency graph
}

void Project::removeResultObject(CustomObject* obj)
{
    int idx = m_resultObjects.indexOf(obj);
    if (idx >= 0) removeResultObject(idx);
}

CustomObject* Project::findObject(const QUuid& uuid) const
{
    for (auto* obj : m_dataObjects)
        if (obj->uuid() == uuid) return obj;
    for (auto* obj : m_resultObjects)
        if (obj->uuid() == uuid) return obj;
    return nullptr;
}

CustomObject* Project::findObject(const QString& name) const
{
    for (auto* obj : m_dataObjects)
        if (obj->name() == name) return obj;
    for (auto* obj : m_resultObjects)
        if (obj->name() == name) return obj;
    return nullptr;
}

int Project::findObjectIndex(const CustomObject* obj) const
{
    int idx = m_dataObjects.indexOf(const_cast<CustomObject*>(obj));
    if (idx >= 0) return idx;
    return m_resultObjects.indexOf(const_cast<CustomObject*>(obj));
}

bool Project::isNameInUse(const QString& name) const
{
    return findObject(name) != nullptr || isOperationNameInUse(name);
}

bool Project::isUUIDInUse(const QUuid& uuid) const
{
    return findObject(uuid) != nullptr;
}

const QVector<CustomOperation*>& Project::operations() const { return m_operations; }

void Project::addOperation(CustomOperation* op)
{
    if (!op || m_operations.contains(op)) return;
    // Allow operations with the same name as existing ones (auto-rename on result collision
    // is handled by addResultObject; operations themselves do not require unique names)
    op->setParent(this);
    int idx = m_operations.size();
    m_operations.append(op);
    emit operationAdded(op, idx);
}

void Project::removeOperation(int index)
{
    if (index < 0 || index >= m_operations.size()) return;
    CustomOperation* op = m_operations.takeAt(index);
    emit operationRemoved(op, index);
    delete op;  // immediate — commands may hold QPointer<CustomOperation>
}

void Project::removeOperation(CustomOperation* op)
{
    int idx = m_operations.indexOf(op);
    if (idx >= 0) removeOperation(idx);
}

CustomOperation* Project::takeOperation(CustomOperation* op)
{
    int idx = m_operations.indexOf(op);
    if (idx < 0) return nullptr;
    CustomOperation* taken = m_operations.takeAt(idx);
    emit operationRemoved(taken, idx);
    return taken;
}

bool Project::isOperationNameInUse(const QString& name) const
{
    for (const auto* op : m_operations)
        if (op->name() == name) return true;
    return false;
}

void Project::setScaleX(double min, double max) { m_scaleXMin = min; m_scaleXMax = max; emit scaleChanged(); }
double Project::scaleXMin() const { return m_scaleXMin; }
double Project::scaleXMax() const { return m_scaleXMax; }

void Project::setScaleY(double min, double max) { m_scaleYMin = min; m_scaleYMax = max; emit scaleChanged(); }
double Project::scaleYMin() const { return m_scaleYMin; }
double Project::scaleYMax() const { return m_scaleYMax; }

void Project::setAutoScaleX(bool autoScale) { m_autoScaleX = autoScale; }
bool Project::autoScaleX() const { return m_autoScaleX; }

void Project::setAutoScaleY(bool autoScale) { m_autoScaleY = autoScale; }
bool Project::autoScaleY() const { return m_autoScaleY; }

void Project::setAspectRatioMode(AspectRatioMode mode) { m_aspectRatioMode = mode; }
Project::AspectRatioMode Project::aspectRatioMode() const { return m_aspectRatioMode; }

void Project::clearAll()
{
    clearOperationList(m_operations);
    clearObjectList(m_resultObjects);
    clearObjectList(m_dataObjects);
    emit projectCleared();
}

void Project::clearObjectList(QVector<CustomObject*>& list)
{
    for (auto* obj : list) obj->deleteLater();
    list.clear();
}

void Project::clearOperationList(QVector<CustomOperation*>& list)
{
    for (auto* op : list) op->deleteLater();
    list.clear();
}

void Project::saveToJson(QJsonObject& json) const
{
    BaseObject::saveToJson(json);

    json[QStringLiteral("scaleXMin")] = m_scaleXMin;
    json[QStringLiteral("scaleXMax")] = m_scaleXMax;
    json[QStringLiteral("scaleYMin")] = m_scaleYMin;
    json[QStringLiteral("scaleYMax")] = m_scaleYMax;
    json[QStringLiteral("autoScaleX")] = m_autoScaleX;
    json[QStringLiteral("autoScaleY")] = m_autoScaleY;
    json[QStringLiteral("aspectRatioMode")] = static_cast<int>(m_aspectRatioMode);

    QJsonArray dataArr;
    for (const auto* obj : m_dataObjects) {
        QJsonObject objJson;
        obj->saveToJson(objJson);
        dataArr.append(objJson);
    }
    json[QStringLiteral("dataObjects")] = dataArr;

    QJsonArray resultArr;
    for (const auto* obj : m_resultObjects) {
        QJsonObject objJson;
        obj->saveToJson(objJson);
        resultArr.append(objJson);
    }
    json[QStringLiteral("resultObjects")] = resultArr;

    QJsonArray opArr;
    for (const auto* op : m_operations) {
        QJsonObject opJson;
        op->saveToJson(opJson);
        opArr.append(opJson);
    }
    json[QStringLiteral("operations")] = opArr;
}

void Project::loadFromJson(const QJsonObject& json)
{
    BaseObject::loadFromJson(json);
    clearAll();

    m_scaleXMin = json[QStringLiteral("scaleXMin")].toDouble(-10.0);
    m_scaleXMax = json[QStringLiteral("scaleXMax")].toDouble(10.0);
    m_scaleYMin = json[QStringLiteral("scaleYMin")].toDouble(-10.0);
    m_scaleYMax = json[QStringLiteral("scaleYMax")].toDouble(10.0);
    m_autoScaleX = json[QStringLiteral("autoScaleX")].toBool(true);
    m_autoScaleY = json[QStringLiteral("autoScaleY")].toBool(true);
    m_aspectRatioMode = static_cast<AspectRatioMode>(json[QStringLiteral("aspectRatioMode")].toInt(0));

    for (const auto& val : json[QStringLiteral("dataObjects")].toArray()) {
        CustomObject* obj = ObjectFactory::createFromJson(val.toObject(), this);
        if (obj) m_dataObjects.append(obj);
    }
    for (const auto& val : json[QStringLiteral("resultObjects")].toArray()) {
        CustomObject* obj = ObjectFactory::createFromJson(val.toObject(), this);
        if (obj) m_resultObjects.append(obj);
    }
    for (const auto& val : json[QStringLiteral("operations")].toArray()) {
        CustomOperation* op = ObjectFactory::createOperationFromJson(val.toObject(), this);
        if (op) m_operations.append(op);
    }
}

} // namespace ExpressDesigner