#pragma once

#include "CustomOperation.h"

namespace ExpressDesigner {

class CarthesianOvalOperation : public CustomOperation {
    Q_OBJECT

public:
    explicit CarthesianOvalOperation(const QString& name = QString(),
                                     QObject* parent = nullptr);
    ~CarthesianOvalOperation() override;

    bool execute(Project* project) override;
    bool isParamObject(int index) const override;
    QString resultName() const override;
    QString paramPrefixOnTree(int index) const override;

    void saveToJson(QJsonObject& json) const override;
    void loadFromJson(const QJsonObject& json) override;

    static constexpr int PARAM_WF1 = 0;
    static constexpr int PARAM_WF2 = 1;
    static constexpr int PARAM_REF_POINT = 2;
    static constexpr int PARAM_RESULT = 3;
};

} // namespace ExpressDesigner