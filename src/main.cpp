#include <QApplication>
#include <QIcon>
#include <QCommandLineParser>
#include <QDebug>
#include "app/OvalDesignerApp.h"
#include <utils/Logger.h>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("2D Express Designer"));
    app.setApplicationVersion(QStringLiteral("4.0.0"));
    app.setOrganizationName(QStringLiteral("ExpressDesignerTeam"));
    app.setOrganizationDomain(QStringLiteral("expressdesigner.io"));

    app.setWindowIcon(QIcon(":/resources/ExpressDesigner_icon.png"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("2D Optical lens design tool"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("project"), QStringLiteral("Project file (.json) to open"));
    parser.process(app);

    ExpressDesigner::Logger::instance().setFile(
        QCoreApplication::applicationDirPath() + QStringLiteral("/ExpressDesigner.log"));

    ExpressDesigner::ExpressDesignerApp designerApp;
    designerApp.initialize();

    const QStringList args = parser.positionalArguments();
    if (!args.isEmpty()) {
        designerApp.openProject(args.first());
    }

    return app.exec();
}