#pragma once
#include <QString>
#include <QVector>
#include <QPointF>

namespace ExpressDesigner {

struct CADExportParams {
    QString filePath;
    QVector<QPointF> controlPoints;
    bool wiresOnly = false;
    bool rotational = false;
    QString rotationalAxis;       // "X", "Y", or "Z"
    double angleStart = 0.0;     // degrees
    double angleEnd = 360.0;     // degrees
    int angularSteps = 36;
    bool linear = false;
    QString linearDirection;     // "X", "Y", or "Z"
    double wideness = 1.0;       // mm
};

class CADExporter {
public:
    /// Exports 2D control points as a CAD file (STEP or IGES).
    /// Returns true on success, false on failure.
    /// On failure, errorMessage() provides details.
    static bool exportToCAD(const CADExportParams& params);

    /// Exports multiple objects into a single CAD file.
    static bool exportMultipleToCAD(const QVector<CADExportParams>& allParams);

    /// Returns the last error message (empty if no error).
    static QString errorMessage();

private:
    static QString s_lastError;

    // Helpers
    static bool isStepFile(const QString& path);
    static double toRadians(double degrees);
};

} // namespace ExpressDesigner