#include "Process/ProcessSimulator.h"

#include <filesystem>
#include <iostream>

ProcessSimulator::ProcessSimulator(
    int nx,
    int ny,
    int nz,
    double dx,
    double dy,
    double dz,
    MaterialType initialMaterial,
    int substrateLayers
)
    : mesh_(nx, ny, nz, dx, dy, dz, initialMaterial, substrateLayers)
{
}

void ProcessSimulator::RunSimpleProcess(const SimulationConfig& config)
{
    std::cout << "\n==============================\n";
    std::cout << "Process Simulation Start\n";
    std::cout << "==============================\n";

    std::cout << "\nInitial material: "
              << MaterialTypeToString(config.initialMaterial)
              << "\n";

    std::cout << "\nInitial mesh state\n";
    mesh_.PrintSummary();
    mesh_.PrintAllSlicesZ();

    int stepNumber = 1;

    const bool useLithography = config.resistThickness > 0.0;

    if (useLithography)
    {
        std::cout << "\nStep " << stepNumber++
                  << ": Photolithography - coat "
                  << config.resistThickness
                  << " nm photoresist, expose and develop mask window\n";

        photolithography_.PatternBoxRegion(
            mesh_,
            config.regionIStart,
            config.regionIEnd,
            config.regionJStart,
            config.regionJEnd,
            config.resistThickness
        );

        mesh_.PrintSummary();
        mesh_.PrintAllSlicesZ();
    }

    if (config.etchDepth > 0.0)
    {
        std::cout << "\nStep " << stepNumber++
                  << ": Etch center region by "
                  << config.etchDepth
                  << " nm\n";

        etcher_.EtchBoxRegion(
            mesh_,
            config.regionIStart,
            config.regionIEnd,
            config.regionJStart,
            config.regionJEnd,
            config.etchDepth
        );

        mesh_.PrintSummary();
        mesh_.PrintAllSlicesZ();
    }

    if (config.etchDepth2 > 0.0)
    {
        std::cout << "\nStep " << stepNumber++
                  << ": Additional etching center region by "
                  << config.etchDepth2
                  << " nm\n";

        etcher_.EtchBoxRegion(
            mesh_,
            config.regionIStart,
            config.regionIEnd,
            config.regionJStart,
            config.regionJEnd,
            config.etchDepth2
        );

        mesh_.PrintSummary();
        mesh_.PrintAllSlicesZ();
    }

    if (config.depositDepth > 0.0)
    {
        std::cout << "\nStep " << stepNumber++
                  << ": Deposit oxide by "
                  << config.depositDepth
                  << " nm into etched region\n";

        depositor_.DepositBoxRegion(
            mesh_,
            config.regionIStart,
            config.regionIEnd,
            config.regionJStart,
            config.regionJEnd,
            config.depositDepth,
            MaterialType::Oxide
        );

        mesh_.PrintSummary();
        mesh_.PrintAllSlicesZ();
    }

    if (useLithography)
    {
        std::cout << "\nStep " << stepNumber++
                  << ": Strip remaining photoresist (ashing)\n";

        photolithography_.StripResist(mesh_);

        mesh_.PrintSummary();
        mesh_.PrintAllSlicesZ();
    }

    if (stepNumber == 1)
    {
        std::cout << "\nNo process steps were executed "
                  << "(all depths are zero).\n";
    }

    std::cout << "\nProcess Simulation Finished\n";
}

void ProcessSimulator::ExportResult(const std::string& filename) const
{
    csvExporter_.ExportMeshGrid(mesh_, filename);

    std::cout << "\nCSV result exported to: "
              << filename
              << "\n";

    const std::string vtkFilename =
        std::filesystem::path(filename)
            .replace_extension(".vtk")
            .string();

    vtkExporter_.ExportMeshGrid(mesh_, vtkFilename);

    std::cout << "VTK result exported to: "
              << vtkFilename
              << " (open with ParaView)\n";

    std::filesystem::path geometryPath(filename);
    geometryPath.replace_filename(
        geometryPath.stem().string() + "_geometry.vtk"
    );

    const std::string geometryFilename = geometryPath.string();

    vtkExporter_.ExportMaterialGeometry(mesh_, geometryFilename);

    std::cout << "VTK geometry exported to: "
              << geometryFilename
              << " (open with ParaView, real topography)\n";
}
