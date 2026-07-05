#pragma once
#include <QString>
#include <QVector>
#include <QPointF>

namespace ExpressDesigner {

struct CADExportParams; // forward declaration from CADExporter.h

/// Lightweight STL exporter — no external dependencies (no OpenCASCADE).
/// Works on Windows, macOS, Linux, and Android.
class STLExporter {
public:
    /// Export a single 2D profile (with optional revolution/extrusion) as an STL mesh.
    /// Returns true on success, false on failure.
    /// On failure, errorMessage() provides details.
    static bool exportToSTL(const CADExportParams& params);

    /// Export multiple 2D profiles into a single STL file.
    static bool exportMultipleToSTL(const QVector<CADExportParams>& allParams);

    /// Returns the last error message (empty if no error).
    static QString errorMessage();

private:
    static QString s_lastError;

    struct Vec3 {
        float x, y, z;
        Vec3() : x(0), y(0), z(0) {}
        Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
    };

    struct Triangle {
        Vec3 v0, v1, v2;
        Vec3 normal;
    };

    /// Write binary STL (compact, standard format).
    static bool writeBinarySTL(const QString& path, const QVector<Triangle>& triangles);

    /// Write ASCII STL (human-readable, larger).
    static bool writeAsciiSTL(const QString& path, const QVector<Triangle>& triangles);

    /// Revolve a 2D polyline around an axis → generate triangles.
    static QVector<Triangle> revolveCurve(const QVector<QPointF>& pts,
                                          const QString& axis,
                                          double angleStartDeg,
                                          double angleEndDeg,
                                          int angularSteps);

    /// Extrude a 2D polyline along a vector → generate triangles.
    static QVector<Triangle> extrudeCurve(const QVector<QPointF>& pts,
                                          const QString& direction,
                                          double distance);

    /// Flat 2D triangulation (fan from centroid) — for wires-only mode.
    static QVector<Triangle> triangulateFlat(const QVector<QPointF>& pts, float z);

    /// Compute triangle normal (right-hand rule).
    static Vec3 computeNormal(const Vec3& a, const Vec3& b, const Vec3& c);
};

} // namespace ExpressDesigner