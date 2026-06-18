#include <QApplication>
#include <QIcon>
#include <QPainter>
#include <QPixmap>
#include <QCommandLineParser>
#include <QDebug>
#include "app/OvalDesignerApp.h"
#include <utils/Logger.h>

static QIcon createAppIcon()
{
    // Generate a "2D Express Designer" icon programmatically
    QPixmap pix(256, 256);
    pix.fill(Qt::transparent);

    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing);

    // Background rounded rectangle
    painter.setBrush(QColor(30, 60, 120));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(8, 8, 240, 240, 40, 40);

    // Inner lighter panel
    painter.setBrush(QColor(45, 85, 160));
    painter.drawRoundedRect(24, 24, 224, 224, 32, 32);

    // "2D" text
    QFont font;
    font.setPixelSize(72);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(QRect(24, 30, 224, 90), Qt::AlignCenter, QStringLiteral("2D"));

    // Subtitle "Express" below 2D
    font.setPixelSize(26);
    font.setBold(false);
    painter.setFont(font);
    painter.drawText(QRect(24, 100, 224, 40), Qt::AlignCenter, QStringLiteral("EXPRESS"));

    // "Designer" bottom
    font.setPixelSize(20);
    painter.setFont(font);
    painter.drawText(QRect(24, 140, 224, 40), Qt::AlignCenter, QStringLiteral("DESIGNER"));

    // Optical beam/lens graphic at bottom
    painter.setBrush(Qt::NoBrush);
    QPen pen(QColor(100, 200, 255), 3);
    painter.setPen(pen);
    // Horizontal guideline
    painter.drawLine(60, 220, 196, 220);
    // Lens shape (two arcs)
    painter.drawArc(QRect(90, 190, 35, 80), 90 * 16, 180 * 16);
    painter.drawArc(QRect(131, 190, 35, 80), 270 * 16, 180 * 16);
    // Arrow points
    painter.drawLine(55, 220, 75, 220);
    painter.drawLine(65, 215, 75, 220);
    painter.drawLine(65, 225, 75, 220);
    painter.drawLine(180, 220, 201, 220);
    painter.drawLine(191, 215, 181, 220);
    painter.drawLine(191, 225, 181, 220);

    painter.end();
    return QIcon(pix);
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("2D Express Designer"));
    app.setApplicationVersion(QStringLiteral("4.0.0"));
    app.setOrganizationName(QStringLiteral("ExpressDesignerTeam"));
    app.setOrganizationDomain(QStringLiteral("expressdesigner.io"));

    app.setWindowIcon(createAppIcon());

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("2D Optical lens design tool"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("project"), QStringLiteral("Project file (.json) to open"));
    parser.process(app);

    ExpressDesigner::Logger::instance().setFile(QStringLiteral("ExpressDesigner.log"));

    ExpressDesigner::ExpressDesignerApp designerApp;
    designerApp.initialize();

    const QStringList args = parser.positionalArguments();
    if (!args.isEmpty()) {
        designerApp.openProject(args.first());
    }

    return app.exec();
}