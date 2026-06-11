#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include "app/OvalDesignerApp.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("ExpressDesigner"));
    app.setApplicationVersion(QStringLiteral("4.0.0"));
    app.setOrganizationName(QStringLiteral("ExpressDesignerTeam"));
    app.setOrganizationDomain(QStringLiteral("expressdesigner.io"));

    // Command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Optical lens design tool"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("project"), QStringLiteral("Project file (.json) to open"));
    parser.process(app);

    ExpressDesigner::ExpressDesignerApp designerApp;
    designerApp.initialize();

    const QStringList args = parser.positionalArguments();
    if (!args.isEmpty()) {
        designerApp.openProject(args.first());
    }

    return app.exec();
}