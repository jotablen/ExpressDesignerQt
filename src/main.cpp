#include <QApplication>
#include <QIcon>
#include <QCommandLineParser>
#include <QDebug>
#include "app/OvalDesignerApp.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("2D Express Designer"));
    app.setApplicationVersion(QStringLiteral("4.0.0"));
    app.setOrganizationName(QStringLiteral("ExpressDesignerTeam"));
    app.setOrganizationDomain(QStringLiteral("expressdesigner.io"));

    // 2D Express Designer icon
    QIcon appIcon;
    appIcon.addFile(QStringLiteral(":/icons/app_16.png"), QSize(16, 16));
    appIcon.addFile(QStringLiteral(":/icons/app_24.png"), QSize(24, 24));
    appIcon.addFile(QStringLiteral(":/icons/app_32.png"), QSize(32, 32));
    appIcon.addFile(QStringLiteral(":/icons/app_48.png"), QSize(48, 48));
    appIcon.addFile(QStringLiteral(":/icons/app_256.png"), QSize(256, 256));
    app.setWindowIcon(appIcon);

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("2D Optical lens design tool"));
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