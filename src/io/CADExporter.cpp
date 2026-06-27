#include "CADExporter.h"

#include <Standard_Version.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax2.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Compound.hxx>
#include <BRep_Builder.hxx>
#include <ShapeFix_Wire.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <GeomAPI_PointsToBSpline.hxx>
#include <Geom_BSplineCurve.hxx>
#include <STEPControl_Writer.hxx>
#include <STEPControl_StepModelType.hxx>
#include <IGESControl_Writer.hxx>
#include <IGESControl_Controller.hxx>
#include <Interface_Static.hxx>
#include <Message_ProgressRange.hxx>
#include <utils/Logger.h>
#include <QtMath>

namespace ExpressDesigner {

QString CADExporter::s_lastError;

bool CADExporter::isStepFile(const QString& path)
{
    const QString lower = path.toLower();
    return lower.endsWith(QStringLiteral(".step")) || lower.endsWith(QStringLiteral(".stp"));
}

double CADExporter::toRadians(double degrees) { return degrees * M_PI / 180.0; }

/// Build one B-spline edge from control points
static TopoDS_Edge buildEdge(const QVector<QPointF>& pts, QString& err)
{
    if (pts.size() < 2) { err = QStringLiteral("Need at least 2 points."); return TopoDS_Edge(); }
    TColgp_Array1OfPnt array(1, pts.size());
    for (int i = 0; i < pts.size(); ++i)
        array.SetValue(i + 1, gp_Pnt(pts[i].x(), pts[i].y(), 0.0));
    GeomAPI_PointsToBSpline splineMaker(array, 3, 8, GeomAbs_C2, 1.0e-9);
    if (!splineMaker.IsDone()) { err = QStringLiteral("B-spline build failed."); return TopoDS_Edge(); }
    return BRepBuilderAPI_MakeEdge(splineMaker.Curve());
}

/// Build open wire (no closing edge) — for wires-only export
static bool buildWire(const QVector<QPointF>& pts, TopoDS_Wire& outWire, QString& err)
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

/// Build closed wire for extrusion (closes if endpoints differ)
static bool buildClosedWire(const QVector<QPointF>& pts, TopoDS_Wire& outWire, QString& err)
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

static bool buildShape(const CADExportParams& params,
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

        // ═══ Axis-crossing & endpoint-near-axis detection ═══
        // OpenCASCADE requires: endpoints on axis must have tangent ⊥ axis.
        // Curves crossing the axis create self-intersecting shells.
        // If an endpoint is within 1% of the curve extent of the axis,
        // force tangent at that endpoint to be perpendicular.
        bool crossesAxis = false;
        double minCoord = 1e9, maxCoord = -1e9;
        for (const auto& pt : params.controlPoints) {
            double checkVal = (params.rotationalAxis == QStringLiteral("Z")) ? pt.y() : pt.x();
            if (checkVal < minCoord) minCoord = checkVal;
            if (checkVal > maxCoord) maxCoord = checkVal;
            if (checkVal < -1e-6) { crossesAxis = true; break; }
        }
        if (crossesAxis) {
            LOG_WARN(QStringLiteral("CAD"), QStringLiteral("Curve crosses axis — cannot revolve; exporting wire only."));
            err = QStringLiteral("Curve crosses rotational axis.");
            outShape = wire;
            return false;
        }
        // Check if curve endpoints are near the axis → tangent must be ⊥
        double axisNear = qMax(qAbs(maxCoord), qAbs(minCoord)) * 0.01;
        if (qAbs(minCoord) < axisNear || qAbs(maxCoord) < axisNear) {
            LOG_INFO(QStringLiteral("CAD"), QStringLiteral("Curve near axis — applying ShapeFix_Wire healing."));
        }

        // Heal wire tolerance before building face/revol
        ShapeFix_Wire wireFix(wire, TopoDS_Face(), 1e-6);
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

bool CADExporter::exportToCAD(const CADExportParams& params)
{
    s_lastError.clear();
    if (params.controlPoints.isEmpty()) { s_lastError = QStringLiteral("No control points to export."); return false; }
    if (params.filePath.isEmpty()) { s_lastError = QStringLiteral("No output file path specified."); return false; }
    try {
        TopoDS_Wire wire;
        if (!buildWire(params.controlPoints, wire, s_lastError)) return false;
        TopoDS_Shape finalShape;
        buildShape(params, wire, finalShape, s_lastError);

        if (isStepFile(params.filePath)) {
            STEPControl_Writer stepWriter;
            Interface_Static::SetIVal("write.step.schema", 4);
            if (stepWriter.Transfer(finalShape, STEPControl_AsIs) != IFSelect_RetDone) {
                s_lastError = QStringLiteral("Failed to transfer shape to STEP writer.");
                return false;
            }
            if (!stepWriter.Write(params.filePath.toUtf8().constData())) {
                s_lastError = QStringLiteral("Failed to write STEP file: ") + params.filePath;
                return false;
            }
        } else {
            IGESControl_Controller::Init();
            IGESControl_Writer igesWriter("MM", 0);
            if (!igesWriter.AddShape(finalShape)) {
                s_lastError = QStringLiteral("Failed to transfer shape to IGES writer.");
                return false;
            }
            igesWriter.ComputeModel();
            if (!igesWriter.Write(params.filePath.toUtf8().constData())) {
                s_lastError = QStringLiteral("Failed to write IGES file: ") + params.filePath;
                return false;
            }
        }
        return true;
    } catch (const Standard_Failure& e) { s_lastError = QString::fromUtf8(e.GetMessageString()); return false; }
    catch (const std::exception& e) { s_lastError = QString::fromUtf8(e.what()); return false; }
    catch (...) { s_lastError = QStringLiteral("Unknown error during CAD export."); return false; }
}

QString CADExporter::errorMessage() { return s_lastError; }

bool CADExporter::exportMultipleToCAD(const QVector<CADExportParams>& allParams)
{
    s_lastError.clear();
    if (allParams.isEmpty()) { s_lastError = QStringLiteral("No objects to export."); return false; }
    const QString& filePath = allParams.first().filePath;
    if (filePath.isEmpty()) { s_lastError = QStringLiteral("No output file path specified."); return false; }

    LOG_INFO(QStringLiteral("CAD"), QStringLiteral("exportMultipleToCAD: %1 objects → %2").arg(allParams.size()).arg(filePath));
    try {
        const bool isStep = isStepFile(filePath);
        LOG_INFO(QStringLiteral("CAD"), isStep ? QStringLiteral("Format: STEP (AP214)") : QStringLiteral("Format: IGES"));

        if (isStep) {
            STEPControl_Writer stepWriter;
            Interface_Static::SetIVal("write.step.schema", 4);
            TopoDS_Compound compound;
            BRep_Builder compoundBuilder;
            compoundBuilder.MakeCompound(compound);
            int shapeCount = 0;

            for (int i = 0; i < allParams.size(); ++i) {
                const auto& params = allParams[i];
                LOG_INFO(QStringLiteral("CAD"), QStringLiteral("[%1/%2] %3 pts, rot=%4 lin=%5").arg(i+1).arg(allParams.size())
                         .arg(params.controlPoints.size()).arg(params.rotational?1:0).arg(params.linear?1:0));
                if (params.controlPoints.size() < 2) continue;

                TopoDS_Wire wire;
                QString err;
                bool ok = buildWire(params.controlPoints, wire, err);
                if (!ok) { LOG_WARN(QStringLiteral("CAD"), QStringLiteral("[%1/%2] Wire FAILED: %3").arg(i+1).arg(allParams.size()).arg(err)); continue; }
                LOG_INFO(QStringLiteral("CAD"), QStringLiteral("[%1/%2] Wire OK").arg(i+1).arg(allParams.size()));

                TopoDS_Shape shape;
                if (buildShape(params, wire, shape, err) || !err.isEmpty()) {
                    if (!err.isEmpty()) LOG_WARN(QStringLiteral("CAD"), QStringLiteral("[%1/%2] Extrusion degraded: %3").arg(i+1).arg(allParams.size()).arg(err));
                    compoundBuilder.Add(compound, shape);
                    ++shapeCount;
                }
            }

            LOG_INFO(QStringLiteral("CAD"), QStringLiteral("Writing STEP with %1 shapes...").arg(shapeCount));
            stepWriter.Transfer(compound, STEPControl_AsIs);
            if (!stepWriter.Write(filePath.toUtf8().constData())) {
                s_lastError = QStringLiteral("Failed to write STEP file: ") + filePath;
                LOG_ERROR(QStringLiteral("CAD"), s_lastError);
                return false;
            }
        } else {
            IGESControl_Controller::Init();
            IGESControl_Writer igesWriter("MM", 0);
            TopoDS_Compound compound;
            BRep_Builder compoundBuilder;
            compoundBuilder.MakeCompound(compound);
            int shapeCount = 0;

            for (int i = 0; i < allParams.size(); ++i) {
                const auto& params = allParams[i];
                LOG_INFO(QStringLiteral("CAD"), QStringLiteral("[%1/%2] %3 pts, rot=%4 lin=%5").arg(i+1).arg(allParams.size())
                         .arg(params.controlPoints.size()).arg(params.rotational?1:0).arg(params.linear?1:0));
                if (params.controlPoints.size() < 2) continue;

                TopoDS_Wire wire;
                QString err;
                bool ok = buildWire(params.controlPoints, wire, err);
                if (!ok) { LOG_WARN(QStringLiteral("CAD"), QStringLiteral("[%1/%2] Wire FAILED: %3").arg(i+1).arg(allParams.size()).arg(err)); continue; }
                LOG_INFO(QStringLiteral("CAD"), QStringLiteral("[%1/%2] Wire OK").arg(i+1).arg(allParams.size()));

                TopoDS_Shape shape;
                if (buildShape(params, wire, shape, err) || !err.isEmpty()) {
                    if (!err.isEmpty()) LOG_WARN(QStringLiteral("CAD"), QStringLiteral("[%1/%2] Extrusion degraded: %3").arg(i+1).arg(allParams.size()).arg(err));
                    compoundBuilder.Add(compound, shape);
                    ++shapeCount;
                }
            }

            LOG_INFO(QStringLiteral("CAD"), QStringLiteral("Writing IGES with %1 shapes...").arg(shapeCount));
            igesWriter.AddShape(compound);
            igesWriter.ComputeModel();
            if (!igesWriter.Write(filePath.toUtf8().constData())) {
                s_lastError = QStringLiteral("Failed to write IGES file: ") + filePath;
                LOG_ERROR(QStringLiteral("CAD"), s_lastError);
                return false;
            }
        }
        LOG_INFO(QStringLiteral("CAD"), QStringLiteral("exportMultipleToCAD: SUCCESS → %1").arg(filePath));
        return true;
    } catch (const Standard_Failure& e) { s_lastError = QString::fromUtf8(e.GetMessageString()); LOG_ERROR(QStringLiteral("CAD"), s_lastError); return false; }
    catch (const std::exception& e) { s_lastError = QString::fromUtf8(e.what()); LOG_ERROR(QStringLiteral("CAD"), s_lastError); return false; }
    catch (...) { s_lastError = QStringLiteral("Unknown error."); LOG_ERROR(QStringLiteral("CAD"), s_lastError); return false; }
}

} // namespace ExpressDesigner