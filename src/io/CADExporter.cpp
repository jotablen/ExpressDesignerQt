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
#include <QtMath>

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

/// Build a wire from control points. Wire is NOT closed — preserves open curves.
static bool buildWire(const QVector<QPointF>& pts, TopoDS_Wire& outWire, QString& err)
{
    BRepBuilderAPI_MakeWire wireBuilder;
    for (int i = 0; i < pts.size() - 1; ++i) {
        gp_Pnt p1(pts[i].x(), pts[i].y(), 0.0);
        gp_Pnt p2(pts[i + 1].x(), pts[i + 1].y(), 0.0);
        wireBuilder.Add(BRepBuilderAPI_MakeEdge(p1, p2));
    }
    wireBuilder.Build();
    if (!wireBuilder.IsDone()) {
        err = QStringLiteral("Failed to build wire from control points.");
        return false;
    }
    outWire = wireBuilder.Wire();
    return true;
}

/// Build a shape from a wire + extrusion params
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
        BRepBuilderAPI_MakeFace faceMaker(wire);
        faceMaker.Build();
        if (!faceMaker.IsDone()) {
            err = QStringLiteral("Cannot build face from wire; exporting wire only.");
            outShape = wire;
            return true; // degraded but OK
        }

        gp_Ax1 rotAxis;
        if (params.rotationalAxis == QStringLiteral("X"))
            rotAxis = gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0));
        else if (params.rotationalAxis == QStringLiteral("Z"))
            rotAxis = gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
        else
            rotAxis = gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 1, 0));

        double angleRad = (params.angleEnd - params.angleStart) * M_PI / 180.0;
        BRepPrimAPI_MakeRevol revol(faceMaker.Face(), rotAxis, angleRad);
        revol.Build();
        if (!revol.IsDone()) {
            err = QStringLiteral("Rotational extrusion failed; exporting wire only.");
            outShape = wire;
            return true;
        }
        outShape = revol.Shape();
        return true;
    }

    if (params.linear) {
        BRepBuilderAPI_MakeFace faceMaker(wire);
        faceMaker.Build();
        if (!faceMaker.IsDone()) {
            err = QStringLiteral("Cannot build face from wire; exporting wire only.");
            outShape = wire;
            return true;
        }

        gp_Vec vec;
        double len = params.wideness;
        if (params.linearDirection == QStringLiteral("X"))
            vec = gp_Vec(len, 0, 0);
        else if (params.linearDirection == QStringLiteral("Y"))
            vec = gp_Vec(0, len, 0);
        else
            vec = gp_Vec(0, 0, len);

        BRepPrimAPI_MakePrism prism(faceMaker.Face(), vec);
        prism.Build();
        if (!prism.IsDone()) {
            err = QStringLiteral("Linear extrusion failed; exporting wire only.");
            outShape = wire;
            return true;
        }
        outShape = prism.Shape();
        return true;
    }

    outShape = wire;
    return true;
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
        TopoDS_Wire wire;
        if (!buildWire(params.controlPoints, wire, s_lastError))
            return false;

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

bool CADExporter::exportMultipleToCAD(const QVector<CADExportParams>& allParams)
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

    try {
        const bool isStep = isStepFile(filePath);

        if (isStep) {
            STEPControl_Writer stepWriter;
            Interface_Static::SetIVal("write.step.schema", 4);

            for (const auto& params : allParams) {
                if (params.controlPoints.size() < 2) continue;

                TopoDS_Wire wire;
                QString err;
                if (!buildWire(params.controlPoints, wire, err)) continue;

                TopoDS_Shape shape;
                buildShape(params, wire, shape, err);
                stepWriter.Transfer(shape, STEPControl_AsIs);
            }

            if (!stepWriter.Write(filePath.toUtf8().constData())) {
                s_lastError = QStringLiteral("Failed to write STEP file: ") + filePath;
                return false;
            }
        } else {
            IGESControl_Controller::Init();
            IGESControl_Writer igesWriter("MM", 0);

            for (const auto& params : allParams) {
                if (params.controlPoints.size() < 2) continue;

                TopoDS_Wire wire;
                QString err;
                if (!buildWire(params.controlPoints, wire, err)) continue;

                TopoDS_Shape shape;
                buildShape(params, wire, shape, err);
                igesWriter.AddShape(shape);
            }

            igesWriter.ComputeModel();
            if (!igesWriter.Write(filePath.toUtf8().constData())) {
                s_lastError = QStringLiteral("Failed to write IGES file: ") + filePath;
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

} // namespace ExpressDesigner