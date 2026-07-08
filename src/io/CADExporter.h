#pragma once
#include <QString>
#include <QVector>
#include <QPointF>
#include <core/CADGeometryBuilder.h>

namespace ExpressDesigner {

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
};

} // namespace ExpressDesigner