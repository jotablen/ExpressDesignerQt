#include "STLExporter.h"
#include "CADExporter.h"   // for CADExportParams
#include <QFile>
#include <QtMath>
#include <limits>
#include <cstring>

namespace ExpressDesigner {

QString STLExporter::s_lastError;

// ============================================================================
// Vec3 helpers
// ============================================================================
static STLExporter::Vec3 rotateY(const STLExporter::Vec3& p, double angleRad)
{
    double c = qCos(angleRad), s = qSin(angleRad);
    return {static_cast<float>(p.x * c + p.z * s),
            p.y,
            static_cast<float>(-p.x * s + p.z * c)};
}

static STLExporter::Vec3 rotateZ(const STLExporter::Vec3& p, double angleRad)
{
    double c = qCos(angleRad), s = qSin(angleRad);
    return {static_cast<float>(p.x * c - p.y * s),
            static_cast<float>(p.x * s + p.y * c),
            p.z};
}

static STLExporter::Vec3 rotateX(const STLExporter::Vec3& p, double angleRad)
{
    double c = qCos(angleRad), s = qSin(angleRad);
    return {p.x,
            static_cast<float>(p.y * c - p.z * s),
            static_cast<float>(p.y * s + p.z * c)};
}

static STLExporter::Vec3 rotatePoint(const STLExporter::Vec3& p, const QString& axis, double angleRad)
{
    if (axis == QStringLiteral("X"))
        return rotateX(p, angleRad);
    if (axis == QStringLiteral("Y"))
        return rotateY(p, angleRad);
    return rotateZ(p, angleRad); // Z is default
}

// ============================================================================
// Triangulation: revolution
// ============================================================================
QVector<STLExporter::Triangle>
STLExporter::revolveCurve(const QVector<QPointF>& pts,
                          const QString& axis,
                          double angleStartDeg,
                          double angleEndDeg,
                          int angularSteps)
{
    QVector<Triangle> tris;
    if (pts.size() < 2 || angularSteps < 2) return tris;

    double startRad = angleStartDeg * M_PI / 180.0;
    double endRad   = angleEndDeg * M_PI / 180.0;
    double total    = endRad - startRad;

    int nPts = pts.size();
    for (int j = 0; j < angularSteps; ++j) {
        double t0 = startRad + total * static_cast<double>(j) / angularSteps;
        double t1 = startRad + total * static_cast<double>(j + 1) / angularSteps;

        for (int i = 0; i < nPts - 1; ++i) {
            Vec3 p00(static_cast<float>(pts[i].x()),   static_cast<float>(pts[i].y()),   0);
            Vec3 p01(static_cast<float>(pts[i+1].x()), static_cast<float>(pts[i+1].y()), 0);

            Vec3 p10 = rotatePoint(p00, axis, t0);
            Vec3 p11 = rotatePoint(p01, axis, t0);
            Vec3 p20 = rotatePoint(p00, axis, t1);
            Vec3 p21 = rotatePoint(p01, axis, t1);

            // Triangle 1: p10, p11, p21
            Triangle tri1;
            tri1.v0 = p10; tri1.v1 = p11; tri1.v2 = p21;
            tri1.normal = computeNormal(tri1.v0, tri1.v1, tri1.v2);
            tris.append(tri1);

            // Triangle 2: p10, p21, p20
            Triangle tri2;
            tri2.v0 = p10; tri2.v1 = p21; tri2.v2 = p20;
            tri2.normal = computeNormal(tri2.v0, tri2.v1, tri2.v2);
            tris.append(tri2);
        }
    }
    return tris;
}

// ============================================================================
// Triangulation: extrusion
// ============================================================================
QVector<STLExporter::Triangle>
STLExporter::extrudeCurve(const QVector<QPointF>& pts,
                          const QString& direction,
                          double distance)
{
    QVector<Triangle> tris;
    if (pts.size() < 2 || distance < 1e-9) return tris;

    float dx = 0, dy = 0, dz = 0;
    if (direction == QStringLiteral("X")) dx = static_cast<float>(distance);
    else if (direction == QStringLiteral("Y")) dy = static_cast<float>(distance);
    else dz = static_cast<float>(distance);

    int nPts = pts.size();

    // Front and back faces (fan from centroid)
    {
        QVector<STLExporter::Triangle> front = triangulateFlat(pts, 0);
        for (auto& t : front) { tris.append(t); }

        // Move all points by offset and triangulate back face (with reversed winding)
        QVector<QPointF> backPts = pts;
        for (auto& p : backPts) {
            p.setX(p.x() + dx);
            p.setY(p.y() + dy);
        }
        QVector<STLExporter::Triangle> back = triangulateFlat(backPts, dz);
        for (auto& t : back) {
            // Reverse winding for back face
            std::swap(t.v1, t.v2);
            t.normal = computeNormal(t.v0, t.v1, t.v2);
            tris.append(t);
        }
    }

    // Side quads → two triangles each
    for (int i = 0; i < nPts; ++i) {
        int next = (i + 1) % nPts;
        Vec3 b0(static_cast<float>(pts[i].x()),     static_cast<float>(pts[i].y()),     0);
        Vec3 b1(static_cast<float>(pts[next].x()),  static_cast<float>(pts[next].y()),  0);
        Vec3 t0(static_cast<float>(pts[i].x() + dx), static_cast<float>(pts[i].y() + dy), dz);
        Vec3 t1(static_cast<float>(pts[next].x() + dx), static_cast<float>(pts[next].y() + dy), dz);

        Triangle tri1;
        tri1.v0 = b0; tri1.v1 = b1; tri1.v2 = t1;
        tri1.normal = computeNormal(tri1.v0, tri1.v1, tri1.v2);
        tris.append(tri1);

        Triangle tri2;
        tri2.v0 = b0; tri2.v1 = t1; tri2.v2 = t0;
        tri2.normal = computeNormal(tri2.v0, tri2.v1, tri2.v2);
        tris.append(tri2);
    }
    return tris;
}

// ============================================================================
// Flat triangulation (fan from centroid) — for wires-only mode
// ============================================================================
QVector<STLExporter::Triangle>
STLExporter::triangulateFlat(const QVector<QPointF>& pts, float z)
{
    QVector<Triangle> tris;
    int n = pts.size();
    if (n < 3) return tris;

    // Compute centroid
    float cx = 0, cy = 0;
    for (const auto& p : pts) { cx += static_cast<float>(p.x()); cy += static_cast<float>(p.y()); }
    cx /= n;
    cy /= n;
    Vec3 center(cx, cy, z);

    // Fan triangulation
    for (int i = 0; i < n; ++i) {
        int next = (i + 1) % n;
        Vec3 a(static_cast<float>(pts[i].x()),  static_cast<float>(pts[i].y()),  z);
        Vec3 b(static_cast<float>(pts[next].x()), static_cast<float>(pts[next].y()), z);

        Triangle tri;
        tri.v0 = center; tri.v1 = a; tri.v2 = b;
        tri.normal = computeNormal(tri.v0, tri.v1, tri.v2);
        tris.append(tri);
    }
    return tris;
}

// ============================================================================
// Compute normal (right-hand rule)
// ============================================================================
STLExporter::Vec3 STLExporter::computeNormal(const Vec3& a, const Vec3& b, const Vec3& c)
{
    float ux = b.x - a.x, uy = b.y - a.y, uz = b.z - a.z;
    float vx = c.x - a.x, vy = c.y - a.y, vz = c.z - a.z;
    float nx = uy * vz - uz * vy;
    float ny = uz * vx - ux * vz;
    float nz = ux * vy - uy * vx;
    float len = qSqrt(nx * nx + ny * ny + nz * nz);
    if (len > 1e-12f) { nx /= len; ny /= len; nz /= len; }
    return {nx, ny, nz};
}

// ============================================================================
// ASCII STL write
// ============================================================================
bool STLExporter::writeAsciiSTL(const QString& path, const QVector<Triangle>& triangles)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        s_lastError = QStringLiteral("Cannot open file for writing: ") + path;
        return false;
    }
    QTextStream out(&file);
    out << "solid ExpressDesignerExported\n";
    for (const auto& tri : triangles) {
        out << "  facet normal " << tri.normal.x << " " << tri.normal.y << " " << tri.normal.z << "\n";
        out << "    outer loop\n";
        out << "      vertex " << tri.v0.x << " " << tri.v0.y << " " << tri.v0.z << "\n";
        out << "      vertex " << tri.v1.x << " " << tri.v1.y << " " << tri.v1.z << "\n";
        out << "      vertex " << tri.v2.x << " " << tri.v2.y << " " << tri.v2.z << "\n";
        out << "    endloop\n";
        out << "  endfacet\n";
    }
    out << "endsolid ExpressDesignerExported\n";
    file.close();
    return true;
}

// ============================================================================
// Binary STL write
// ============================================================================
bool STLExporter::writeBinarySTL(const QString& path, const QVector<Triangle>& triangles)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        s_lastError = QStringLiteral("Cannot open file for writing: ") + path;
        return false;
    }

    // Header: 80 bytes
    char header[80] = {};
    const char* hdrText = "Binary STL from ExpressDesigner - 2D Optical Lens Design Tool";
    std::strncpy(header, hdrText, 79);
    file.write(header, 80);

    // Triangle count: uint32 (little-endian)
    quint32 count = static_cast<quint32>(triangles.size());
    char countBytes[4];
    countBytes[0] = static_cast<char>(count & 0xFF);
    countBytes[1] = static_cast<char>((count >> 8) & 0xFF);
    countBytes[2] = static_cast<char>((count >> 16) & 0xFF);
    countBytes[3] = static_cast<char>((count >> 24) & 0xFF);
    file.write(countBytes, 4);

    auto writeFloat = [&file](float f) {
        union { float val; quint32 bits; } u;
        u.val = f;
        char b[4];
        b[0] = static_cast<char>(u.bits & 0xFF);
        b[1] = static_cast<char>((u.bits >> 8) & 0xFF);
        b[2] = static_cast<char>((u.bits >> 16) & 0xFF);
        b[3] = static_cast<char>((u.bits >> 24) & 0xFF);
        file.write(b, 4);
    };

    for (const auto& tri : triangles) {
        writeFloat(tri.normal.x);
        writeFloat(tri.normal.y);
        writeFloat(tri.normal.z);
        writeFloat(tri.v0.x); writeFloat(tri.v0.y); writeFloat(tri.v0.z);
        writeFloat(tri.v1.x); writeFloat(tri.v1.y); writeFloat(tri.v1.z);
        writeFloat(tri.v2.x); writeFloat(tri.v2.y); writeFloat(tri.v2.z);
        quint16 attr = 0;
        file.write(reinterpret_cast<const char*>(&attr), 2);
    }

    file.close();
    return true;
}

// ============================================================================
// Public API
// ============================================================================
QString STLExporter::errorMessage() { return s_lastError; }

bool STLExporter::exportToSTL(const CADExportParams& params)
{
    QVector<CADExportParams> all;
    all.append(params);
    return exportMultipleToSTL(all);
}

bool STLExporter::exportMultipleToSTL(const QVector<CADExportParams>& allParams)
{
    s_lastError.clear();
    if (allParams.isEmpty()) {
        s_lastError = QStringLiteral("No objects to export.");
        return false;
    }

    const QString& filePath = allParams.first().filePath;
    if (filePath.isEmpty()) {
        s_lastError = QStringLiteral("No output file path specified.");
        return false;
    }

    QVector<Triangle> allTriangles;

    for (const auto& params : allParams) {
        const auto& pts = params.controlPoints;
        if (pts.size() < 2) continue;

        QVector<Triangle> objTris;

        if (params.rotational && !params.wiresOnly) {
            objTris = revolveCurve(pts, params.rotationalAxis,
                                   params.angleStart,
                                   params.angleEnd,
                                   qMax(4, params.angularSteps));
        } else if (params.linear && !params.wiresOnly) {
            objTris = extrudeCurve(pts, params.linearDirection, params.wideness);
        } else {
            // Wires-only or no extrusion → flat triangulation
            objTris = triangulateFlat(pts, 0);
        }

        allTriangles.append(objTris);
    }

    if (allTriangles.isEmpty()) {
        s_lastError = QStringLiteral("No triangles generated. Check that control points have at least 3 vertices.");
        return false;
    }

    // Use binary format (more compact, standard for CAD tools)
    bool ok = writeBinarySTL(filePath, allTriangles);
    if (!ok) return false;

    s_lastError.clear();
    return true;
}

} // namespace ExpressDesigner