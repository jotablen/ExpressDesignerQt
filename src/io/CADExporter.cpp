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
#include <STEPControl_Writer.hxx>
#include <STEPControl_StepModelType.hxx>
#include <IGESControl_Writer.hxx>
#include <IGESControl_Controller.hxx>
#include <Interface_Static.hxx>
#include <Message_ProgressRange.hxx>

namespace ExpressDesigner {

QString CADExporter::s_lastError;

bool CADExporter::isStepFile(const QString& path)
{
    const QString lower = path.toLower();
    return lower.endsWith(QStringLiteral(".step")) || lower.endsWith(QStringLiteral(".stp"));
}

double CADExporter::toRadians(double degrees)
{
    return degrees * M_PI / 180.0;
}

bool CADExporter::exportToCAD(const CADExportParams& params)
{
    s_lastError.clear();

    if (params.controlPoints.isEmpty()) {
        s_lastError = QStringLiteral("No control points to export.");
        return false;
    }

    if (params.filePath.isEmpty()) {
        s_lastError = QStringLiteral("No output file path specified.");
        return false;
    }

    try {
        // ── Step 1: Build edges from control points ──
        const auto& pts = params.controlPoints;

        // Build edges connecting consecutive points, staying in the XY plane (Z=0)
        TopoDS_Wire wire;
        {
            BRepBuilderAPI_MakeWire wireBuilder;
            for (int i = 0; i < pts.size() - 1; ++i) {
                gp_Pnt p1(pts[i].x(), pts[i].y(), 0.0);
                gp_Pnt p2(pts[i + 1].x(), pts[i + 1].y(), 0.0);
                TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(p1, p2);
                wireBuilder.Add(edge);
            }

            // Close the wire if first and last points are not the same
            if (pts.size() > 1) {
                gp_Pnt first(pts.first().x(), pts.first().y(), 0.0);
                gp_Pnt last(pts.last().x(), pts.last().y(), 0.0);
                if (first.Distance(last) > 1e-9) {
                    TopoDS_Edge closingEdge = BRepBuilderAPI_MakeEdge(last, first);
                    wireBuilder.Add(closingEdge);
                }
            }

            wireBuilder.Build();
            if (!wireBuilder.IsDone()) {
                s_lastError = QStringLiteral("Failed to build wire from control points.");
                return false;
            }
            wire = wireBuilder.Wire();
        }

        TopoDS_Shape finalShape;

        // ── Step 2a: Wires only — just export the wire ──
        if (params.wiresOnly || (!params.rotational && !params.linear)) {
            finalShape = wire;
        }
        // ── Step 2b: Rotational extrusion ──
        else if (params.rotational) {
            // Build a planar face from the wire (needed for revol/prism)
            BRepBuilderAPI_MakeFace faceMaker(wire);
            faceMaker.Build();
            if (!faceMaker.IsDone()) {
                // Try without closing — just export the wire
                s_lastError = QStringLiteral("Cannot build face from wire; exporting wire only.");
                finalShape = wire;
                goto writeFile;
            }
            TopoDS_Face face = faceMaker.Face();

            // Determine rotation axis
            gp_Ax1 rotAxis;
            if (params.rotationalAxis == QStringLiteral("X")) {
                rotAxis = gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0));
            } else if (params.rotationalAxis == QStringLiteral("Z")) {
                rotAxis = gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
            } else {
                // Default: Y axis
                rotAxis = gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 1, 0));
            }

            double angleRad = toRadians(params.angleEnd - params.angleStart);

            BRepPrimAPI_MakeRevol revol(face, rotAxis, angleRad);
            revol.Build();
            if (!revol.IsDone()) {
                s_lastError = QStringLiteral("Rotational extrusion failed; exporting wire only.");
                finalShape = wire;
                goto writeFile;
            }
            finalShape = revol.Shape();
        }
        // ── Step 2c: Linear extrusion ──
        else if (params.linear) {
            // Build a planar face from the wire
            BRepBuilderAPI_MakeFace faceMaker(wire);
            faceMaker.Build();
            if (!faceMaker.IsDone()) {
                s_lastError = QStringLiteral("Cannot build face from wire; exporting wire only.");
                finalShape = wire;
                goto writeFile;
            }
            TopoDS_Face face = faceMaker.Face();

            // Determine extrusion direction vector
            gp_Vec extrudeVec;
            double extrudeLen = params.wideness;
            if (params.linearDirection == QStringLiteral("X")) {
                extrudeVec = gp_Vec(extrudeLen, 0, 0);
            } else if (params.linearDirection == QStringLiteral("Y")) {
                extrudeVec = gp_Vec(0, extrudeLen, 0);
            } else {
                // Default: Z direction
                extrudeVec = gp_Vec(0, 0, extrudeLen);
            }

            BRepPrimAPI_MakePrism prism(face, extrudeVec);
            prism.Build();
            if (!prism.IsDone()) {
                s_lastError = QStringLiteral("Linear extrusion failed; exporting wire only.");
                finalShape = wire;
                goto writeFile;
            }
            finalShape = prism.Shape();
        }

    writeFile:
        // ── Step 3: Write to STEP or IGES ──
        if (isStepFile(params.filePath)) {
            STEPControl_Writer stepWriter;
            // Use manifold solid B-rep mode for better compatibility
            Interface_Static::SetIVal("write.step.schema", 4); // AP214
            if (stepWriter.Transfer(finalShape, STEPControl_AsIs) != IFSelect_RetDone) {
                s_lastError = QStringLiteral("Failed to transfer shape to STEP writer.");
                return false;
            }
            if (!stepWriter.Write(params.filePath.toUtf8().constData())) {
                s_lastError = QStringLiteral("Failed to write STEP file: ") + params.filePath;
                return false;
            }
        } else {
            // IGES
            IGESControl_Controller::Init();
            IGESControl_Writer igesWriter("MM", 0); // MM units, write all entities
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

    } catch (const Standard_Failure& e) {
        s_lastError = QString::fromUtf8(e.GetMessageString());
        return false;
    } catch (const std::exception& e) {
        s_lastError = QString::fromUtf8(e.what());
        return false;
    } catch (...) {
        s_lastError = QStringLiteral("Unknown error during CAD export.");
        return false;
    }
}

QString CADExporter::errorMessage()
{
    return s_lastError;
}

} // namespace ExpressDesigner