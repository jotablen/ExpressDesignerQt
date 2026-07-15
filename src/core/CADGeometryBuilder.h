#pragma once
#include <QString>
#include <QVector>
#include <QPointF>

#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Shape.hxx>

namespace ExpressDesigner {

/// Parameters for CAD extrusion (shared between export and preview)
struct CADExportParams {
    QString filePath;
    QVector<QPointF> controlPoints;
    QVector<QVector<QPointF>> controlPointsList;  // one entry per object for multi-shape preview
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

/**
 * @brief Shared CAD geometry builder using OpenCASCADE.
 *
 * Extracted from CADExporter so that both the exporter and the
 * preview dialog can build the same TopoDS_Shape from the same parameters.
 */
class CADGeometryBuilder {
public:
    /// Build one B-spline edge from 2D control points (Z=0)
    static TopoDS_Edge buildEdge(const QVector<QPointF>& pts, QString& err);

    /// Build open wire (no closing edge)
    static bool buildWire(const QVector<QPointF>& pts, TopoDS_Wire& outWire, QString& err);

    /// Build closed wire (adds closing edge if endpoints differ)
    static bool buildClosedWire(const QVector<QPointF>& pts, TopoDS_Wire& outWire, QString& err);

    /**
     * @brief Build the final 3D shape from wire and extrusion params.
     *
     * If wiresOnly or neither rotational/linear is set, outShape = wire.
     * For rotational: builds a BRepPrimAPI_MakeRevol (face→wire fallback).
     * For linear: builds a BRepPrimAPI_MakePrism (face→wire fallback).
     *
     * @return true if shape built (possibly degraded), false on failure.
     */
    static bool buildShape(const CADExportParams& params,
                           TopoDS_Wire& wire,
                           TopoDS_Shape& outShape,
                           QString& err);

    static bool buildShapeRotational(const CADExportParams& params,
                                     TopoDS_Wire& wire,
                                     TopoDS_Shape& outShape,
                                     QString& err);

    static bool buildShapeLinear(const CADExportParams& params,
                                 TopoDS_Wire& wire,
                                 TopoDS_Shape& outShape,
                                 QString& err);

    /// Convert degrees to radians
    static double toRadians(double degrees) { return degrees * M_PI / 180.0; }
};

} // namespace ExpressDesigner