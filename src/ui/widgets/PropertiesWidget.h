#pragma once
#include <QWidget>
#include <QTabWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QTableWidget>
#include <QListWidget>
#include <QGroupBox>
#include <QLabel>
#include <core/CustomObject.h>
#include <core/Project.h>
#include <core/CustomOperation.h>
#include <core/CommandHistory.h>

namespace ExpressDesigner {

class PropertiesWidget : public QWidget {
    Q_OBJECT
public:
    explicit PropertiesWidget(QWidget* parent = nullptr);
    void setObject(CustomObject* obj);
    void setOperation(CustomOperation* op);
    void refreshGridFromObject(CustomObject* obj);
    void setProject(Project* project);
    void setCommandHistory(CommandHistory* ch) { m_cmdHistory = ch; }
    void setOperations(Project* project);
    void refresh();
    QTabWidget* tabWidget() const { return m_tabs; }

signals:
    void objectModified(CustomObject* obj);
    void projectModified(Project* project);
    void operationModified(CustomOperation* op);
    void insertObjectRequested();
    void deleteObjectRequested();
    void selectObjectRequested(const QString& name);
    void calculateOvalRequested();
    void propagateWFRequested();

private slots:
    void onSaveProject();
    void onRestoreProject();
    void onSavePoint();
    void onRestorePoint();
    void onSaveLine();
    void onRestoreLine();
    void onSaveArc();
    void onRestoreArc();
    void onSaveCurve();
    void onRestoreCurve();
    void onSaveCalcOval();
    void onRestoreCalcOval();
    void onSavePropagate();
    void onRestorePropagate();

private:
    void setupProjectTab();
    void setupPointTab();
    void setupLineTab();
    void setupArcTab();
    void setupCurveTab();
    void setupObjectsTab();
    void setupCalcOvalTab();
    void setupPropagateTab();
    void setupOperationTabs();
    void showObjectTabs(CustomObject* obj);
    void showOperationTabs(CustomOperation* op);
    QGroupBox* createWFPanel(QWidget* parent, QLineEdit*& refIndexEdit, QCheckBox*& flipNCheck);

    QTabWidget* m_tabs = nullptr;
    CustomObject* m_currentObject = nullptr;
    CustomOperation* m_currentOperation = nullptr;
    Project* m_currentProject = nullptr;

    // Project tab widgets
    QLineEdit* m_prjNameEdit = nullptr;
    QLineEdit* m_prjXMinEdit = nullptr;
    QLineEdit* m_prjXMaxEdit = nullptr;
    QLineEdit* m_prjYMinEdit = nullptr;
    QLineEdit* m_prjYMaxEdit = nullptr;
    QCheckBox* m_prjXAutoCheck = nullptr;
    QCheckBox* m_prjYAutoCheck = nullptr;

    // Point tab widgets
    QLineEdit* m_ptNameEdit = nullptr;
    QLineEdit* m_ptRefIndexEdit = nullptr;
    QLineEdit* m_ptXEdit = nullptr;
    QLineEdit* m_ptYEdit = nullptr;
    QCheckBox* m_ptFlipNCheck = nullptr;
    QLabel* m_ptWFStatusLabel = nullptr;
    QGroupBox* m_ptWFPanel = nullptr;

    // Line tab widgets
    QLineEdit* m_lnNameEdit = nullptr;
    QLineEdit* m_lnRefIndexEdit = nullptr;
    QLineEdit* m_lnP1XEdit = nullptr;
    QLineEdit* m_lnP1YEdit = nullptr;
    QLineEdit* m_lnP2XEdit = nullptr;
    QLineEdit* m_lnP2YEdit = nullptr;
    QCheckBox* m_lnFlipNCheck = nullptr;
    QLabel* m_lnWFStatusLabel = nullptr;
    QGroupBox* m_lnWFPanel = nullptr;

    // Arc tab widgets
    QLineEdit* m_arcNameEdit = nullptr;
    QLineEdit* m_arcRefIndexEdit = nullptr;
    QLineEdit* m_arcXEdit = nullptr;
    QLineEdit* m_arcYEdit = nullptr;
    QLineEdit* m_arcRadiusEdit = nullptr;
    QLineEdit* m_arcStartAngleEdit = nullptr;
    QLineEdit* m_arcEndAngleEdit = nullptr;
    QLineEdit* m_arcAmntPtsEdit = nullptr;
    QCheckBox* m_arcFlipNCheck = nullptr;
    QLabel* m_arcWFStatusLabel = nullptr;
    QGroupBox* m_arcWFPanel = nullptr;

    // Curve tab widgets
    QLineEdit* m_cvNameEdit = nullptr;
    QLineEdit* m_cvRefIndexEdit = nullptr;
    QTableWidget* m_cvGrid = nullptr;
    QPushButton* m_cvAddBtn = nullptr;
    QPushButton* m_cvDelBtn = nullptr;
    QPushButton* m_cvEditCurveBtn = nullptr;
    QCheckBox* m_cvFlipNCheck = nullptr;
    CommandHistory* m_cmdHistory = nullptr;
    QLabel* m_cvWFStatusLabel = nullptr;
    QGroupBox* m_cvWFPanel = nullptr;

    // Objects tab widgets
    QListWidget* m_objListWidget = nullptr;

    // CalcOval tab widgets
    QLineEdit* m_cOvalNameEdit = nullptr;
    QLineEdit* m_cOvalWF1Edit = nullptr;
    QLineEdit* m_cOvalWF2Edit = nullptr;
    QLineEdit* m_cOvalRefEdit = nullptr;
    QLineEdit* m_cOvalQtyPtsEdit = nullptr;
    QLineEdit* m_cOvalResultEdit = nullptr;

    // Propagate tab widgets
    QLineEdit* m_propgNameEdit = nullptr;
    QLineEdit* m_propgWFEdit = nullptr;
    QLineEdit* m_propgIOREdit = nullptr;
    QLineEdit* m_propgSurfEdit = nullptr;
    QLineEdit* m_propgQtyPtsEdit = nullptr;
    QLineEdit* m_propgOffsetEdit = nullptr;
    QLineEdit* m_propgResultEdit = nullptr;
};

} // namespace ExpressDesigner