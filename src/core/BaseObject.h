#pragma once

#include <QObject>
#include <QUuid>
#include <QString>
#include <QJsonObject>
#include "ObjectTypes.h"

namespace ExpressDesigner {

/**
 * @brief Base class for all objects in the ExpressDesigner project.
 *
 * Provides common properties: UUID, name, object type, visibility,
 * normal display settings, and serialization interface.
 *
 * Equivalent to TLPE_BaseObject in the original Oval Designer.
 */
class BaseObject : public QObject {
    Q_OBJECT

public:
    explicit BaseObject(ObjectType type, const QString& name = QString(),
                        QObject* parent = nullptr);
    ~BaseObject() override;

    // --- Identity ---
    void setUuid(const QUuid& uuid);
    QUuid uuid() const;

    void setName(const QString& name);
    QString name() const;

    void setObjectType(ObjectType type);
    ObjectType objectType() const;

    // --- Type queries ---
    bool isWavefront() const;
    bool isVirtualWF() const;
    bool isResult() const;
    bool isProject() const;

    // --- Visibility ---
    void setVisible(bool visible);
    bool isVisible() const;

    void setLabelVisible(bool visible);
    bool isLabelVisible() const;

    // --- Normals (Blen 2014) ---
    void setShowNormals(bool show);
    bool showNormals() const;

    void setNormalParams(int raysQty, double raysLen);
    int normalRaysQty() const;
    double normalRaysLen() const;

    void setNormalFlipped(bool flipped);
    bool isNormalFlipped() const;

    void flipNormal();

    // --- Serialization ---
    virtual void saveToJson(QJsonObject& json) const;
    virtual void loadFromJson(const QJsonObject& json);

    // --- Utility ---
    int softwareVersion() const { return m_softwareVersion; }

signals:
    void objectModified(const QString& propertyName);
    void visibilityChanged(bool visible);
    void nameChanged(const QString& newName);

protected:
    QUuid m_uuid;
    QString m_name;
    ObjectType m_type;
    int m_softwareVersion = 4;

    // Visibility
    bool m_visible = true;
    bool m_showLabel = true;

    // Normals
    bool m_showNormals = false;
    int m_raysQty = 10;
    double m_raysLen = 1.0;
    bool m_normalFlipped = false;
};

} // namespace ExpressDesigner