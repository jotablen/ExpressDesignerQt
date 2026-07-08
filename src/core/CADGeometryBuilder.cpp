#include "CADGeometryBuilder.h"

#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax2.hxx>
#include <gp_Vec.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Face.hxx>
#include <ShapeFix_Wire.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <GeomAPI_PointsToBSpline.hxx>
#include <Geom_BSplineCurve.hxx>
#include <utils/Logger.h>
#include <QtMath>

namespace ExpressDesigner {

// ============================================================================
// Static helper — build B-spline edge from 2D control points
// ============================================================================
TopoDS_Edge CADGeometryBuilder::buildEdge(const QVector<QPointF>& pts, QString& err)
{
    if (pts.size() < 2) { err = QStringLiteral("Need at least 2 points."); return TopoDS_Edge(); }
    TColgp_Array1OfPnt array(1, pts.size());
    for (int i = 0; i < pts.size(); ++i)
        array.SetValue(i + 1, gp_Pnt(pts[i].x(), pts[i].y(), 0.0));
    GeomAPI_PointsToBSpline splineMaker(array, 3, 8, GeomAbs_C2, 1.0e-9);
    if (!splineMaker.IsDone()) { err = QStringLiteral("B-spline build failed."); return TopoDS_Edge(); }
    return BRepBuilderAPI_MakeEdge(splineMaker.Curve());
}

// ============================================================================
// buildWire — open wire (no closing edge)
// ============================================================================
bool CADGeometryBuilder::buildWire(const QVector<QPointF>& pts, TopoDS_Wire& outWire, QString& err)
{
    if (pts.size() < 2) { err = QStringLiteral("Need at least 2 control points."); return false; }
    TopoDS_Edge edge = buildEdge(pts, err);
    if (edge.IsNull()) return false;
    BRepBuilderAPI_MakeWire wireBuilder(edge);
    wireBuilder.Build();
    if (!wireBuilder.IsDone()) { err = QStringLiteral("Wire build failed."); return false; }
    outWire = wireBuilder.Wire();
    return true;
}

// ============================================================================
// buildClosedWire — adds closing edge if endpoints differ
// ============================================================================
bool CADGeometryBuilder::buildClosedWire(const QVector<QPointF>& pts, TopoDS_Wire& outWire, QString& err)
{
    if (pts.size() < 2) { err = QStringLiteral("Need at least 2 control points."); return false; }
    TopoDS_Edge edge = buildEdge(pts, err);
    if (edge.IsNull()) return false;
    BRepBuilderAPI_MakeWire wireBuilder(edge);

    // Check if first and last points coincide — if not, add closing straight edge
    gp_Pnt first(pts.first().x(), pts.first().y(), 0.0);
    gp_Pnt last(pts.last().x(), pts.last().y(), 0.0);
    if (first.Distance(last) > 1e-9) {
        TopoDS_Edge closingEdge = BRepBuilderAPI_MakeEdge(last, first);
        wireBuilder.Add(closingEdge);
    }

    wireBuilder.Build();
    if (!wireBuilder.IsDone()) { err = QStringLiteral("Closed wire build failed."); return false; }
    outWire = wireBuilder.Wire();
    return true;
}

// ============================================================================
// buildShape — builds 3D shape from wire + extrusion params
// ============================================================================
bool CADGeometryBuilder::buildShape(const CADExportParams& params,
                                    TopoDS_Wire& wire,
                                    TopoDS_Shape& outShape,
                                    QString& err)
{
    if (params.wiresOnly || (!params.rotational && !params.linear)) {
        outShape = wire;
        return true;
    }

    if (params.rotational) {
        gp_Ax1 rotAxis;
        if (params.rotationalAxis == QStringLiteral("X"))
            rotAxis = gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0));
        else if (params.rotationalAxis == QStringLiteral("Z"))
            rotAxis = gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
        else
            rotAxis = gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 1, 0));
        double angleRad = (params.angleEnd - params.angleStart) * M_PI / 180.0;

        // Re-sample for X-positive curves (rotational)
        // If curve crosses or touches the axis, re-sample to X≥0 portion
        QVector<QPointF> filteredPts;
        for (const auto& pt : params.controlPoints) {
            double checkVal = (params.rotationalAxis == QStringLiteral("Z")) ? pt.y() : pt.x();
            if (checkVal >= -1e-6) filteredPts.append(pt);
        }
        if (filteredPts.size() != params.controlPoints.size()) {
            LOG_INFO(QStringLiteral("CAD"), QStringLiteral("Curve clipped for rotation (%1 pts → %2 pts)")
                     .arg(params.controlPoints.size()).arg(filteredPts.size()));
        }
        if (filteredPts.size() < 2) {
            LOG_WARN(QStringLiteral("CAD"), QStringLiteral("Not enough points after axis clipping; exporting wire only."));
            err = QStringLiteral("Not enough points after axis clipping.");
            outShape = wire;
            return false;
        }
        // Always rebuild wire from (possibly filtered) points
        TopoDS_Wire clippedWire;
        if (!buildWire(filteredPts, clippedWire, err)) {
            outShape = wire;
            return false;
        }
        TopoDS_Wire& workWire = clippedWire;

        // Heal wire tolerance before building face/revol
        ShapeFix_Wire wireFix(workWire, TopoDS_Face(), 1e-6);
        wireFix.Perform();
        TopoDS_Wire healedWire = wireFix.Wire();

        // Try face first, then wire directly
        BRepBuilderAPI_MakeFace faceMaker(healedWire);
        faceMaker.Build();
        if (faceMaker.IsDone()) {
            BRepPrimAPI_MakeRevol revol(faceMaker.Face(), rotAxis, angleRad);
            revol.Build();
            if (revol.IsDone()) { outShape = revol.Shape(); return true; }
            LOG_INFO(QStringLiteral("CAD"), QStringLiteral("Revol with face failed, trying healed wire..."));
        }
        // Fallback: extrude healed wire directly → produces shell
        BRepPrimAPI_MakeRevol revolFromWire(healedWire, rotAxis, angleRad);
        revolFromWire.Build();
        if (revolFromWire.IsDone()) { outShape = revolFromWire.Shape(); return true; }
        err = QStringLiteral("Rotational extrusion failed.");
        outShape = wire;
        return false;
    }

    if (params.linear) {
        gp_Vec vec;
        double len = params.wideness;
        if (params.linearDirection == QStringLiteral("X")) vec = gp_Vec(len, 0, 0);
        else if (params.linearDirection == QStringLiteral("Y")) vec = gp_Vec(0, len, 0);
        else vec = gp_Vec(0, 0, len);

        // Try face first, then wire directly
        BRepBuilderAPI_MakeFace faceMaker(wire);
        faceMaker.Build();
        if (faceMaker.IsDone()) {
            BRepPrimAPI_MakePrism prism(faceMaker.Face(), vec);
            prism.Build();
            if (prism.IsDone()) { outShape = prism.Shape(); return true; }
            LOG_INFO(QStringLiteral("CAD"), QStringLiteral("Prism with face failed, trying wire..."));
        }
        // Fallback: extrude wire directly → produces shell
        BRepPrimAPI_MakePrism prismFromWire(wire, vec);
        prismFromWire.Build();
        if (prismFromWire.IsDone()) { outShape = prismFromWire.Shape(); return true; }
        err = QStringLiteral("Linear extrusion failed.");
        outShape = wire;
        return false;
    }
    outShape = wire;
    return true;
}

} // namespace ExpressDesigner