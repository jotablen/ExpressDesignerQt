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
#include <core/CADGeometryBuilder.h>
#include <utils/Logger.h>
#include <QtMath>

namespace ExpressDesigner {

QString CADExporter::s_lastError;

bool CADExporter::isStepFile(const QString& path)
{
    const QString lower = path.toLower();
    return lower.endsWith(QStringLiteral(".step")) || lower.endsWith(QStringLiteral(".stp"));
}

bool CADExporter::exportToCAD(const CADExportParams& params)
{
    s_lastError.clear();
    if (params.controlPoints.isEmpty()) { s_lastError = QStringLiteral("No control points to export."); return false; }
    if (params.filePath.isEmpty()) { s_lastError = QStringLiteral("No output file path specified."); return false; }
    try {
        TopoDS_Wire wire;
        if (!CADGeometryBuilder::buildWire(params.controlPoints, wire, s_lastError)) return false;
        TopoDS_Shape finalShape;
        CADGeometryBuilder::buildShape(params, wire, finalShape, s_lastError);

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
                const auto& p = allParams[i];
                LOG_INFO(QStringLiteral("CAD"), QStringLiteral("[%1/%2] %3 pts, rot=%4 lin=%5").arg(i+1).arg(allParams.size())
                         .arg(p.controlPoints.size()).arg(p.rotational?1:0).arg(p.linear?1:0));
                if (p.controlPoints.size() < 2) continue;

                TopoDS_Wire wire;
                QString err;
                bool ok = CADGeometryBuilder::buildWire(p.controlPoints, wire, err);
                if (!ok) { LOG_WARN(QStringLiteral("CAD"), QStringLiteral("[%1/%2] Wire FAILED: %3").arg(i+1).arg(allParams.size()).arg(err)); continue; }
                LOG_INFO(QStringLiteral("CAD"), QStringLiteral("[%1/%2] Wire OK").arg(i+1).arg(allParams.size()));

                TopoDS_Shape shape;
                if (CADGeometryBuilder::buildShape(p, wire, shape, err) || !err.isEmpty()) {
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
                const auto& p = allParams[i];
                LOG_INFO(QStringLiteral("CAD"), QStringLiteral("[%1/%2] %3 pts, rot=%4 lin=%5").arg(i+1).arg(allParams.size())
                         .arg(p.controlPoints.size()).arg(p.rotational?1:0).arg(p.linear?1:0));
                if (p.controlPoints.size() < 2) continue;

                TopoDS_Wire wire;
                QString err;
                bool ok = CADGeometryBuilder::buildWire(p.controlPoints, wire, err);
                if (!ok) { LOG_WARN(QStringLiteral("CAD"), QStringLiteral("[%1/%2] Wire FAILED: %3").arg(i+1).arg(allParams.size()).arg(err)); continue; }
                LOG_INFO(QStringLiteral("CAD"), QStringLiteral("[%1/%2] Wire OK").arg(i+1).arg(allParams.size()));

                TopoDS_Shape shape;
                if (CADGeometryBuilder::buildShape(p, wire, shape, err) || !err.isEmpty()) {
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