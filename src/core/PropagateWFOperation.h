#pragma once

#include "CustomOperation.h"

namespace ExpressDesigner {

class PropagateWFOperation : public CustomOperation {
    Q_OBJECT

public:
    explicit PropagateWFOperation(const QString& name = QString(),
                                  QObject* parent = nullptr);
    ~PropagateWFOperation() override;

    bool execute(Project* project) override;
    bool isParamObject(int index) const override;
    QString resultName() const override;
    QString paramPrefixOnTree(int index) const override;

    double offset() const;
    void setOffset(double value);

    void saveToJson(QJsonObject& json) const override;
    void loadFromJson(const QJsonObject& json) override;

    static constexpr int PARAM_WF = 0;
    static constexpr int PARAM_SURFACE = 1;
    static constexpr int PARAM_IOR = 2;
    static constexpr int PARAM_OFFSET = 3;
    static constexpr int PARAM_RESULT = 4;

private:
    double m_offset = 0.0;
};

} // namespace ExpressDesigner