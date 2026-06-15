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

void ProcessSimulator::RunProcess(const SimulationConfig& config)
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

    if (config.steps.empty())
    {
        std::cout << "\nNo process steps were defined.\n";
        std::cout << "\nProcess Simulation Finished\n";
        return;
    }

    int stepNumber = 1;

    for (const ProcessStep& step : config.steps)
    {
        std::cout << "\nStep " << stepNumber++ << ": ";
        ApplyStep(step);

        mesh_.PrintSummary();

        if (config.verbose)
        {
            mesh_.PrintAllSlicesZ();
        }
    }

    if (!config.verbose)
    {
        std::cout << "\nFinal mesh state\n";
        mesh_.PrintAllSlicesZ();
    }

    std::cout << "\nProcess Simulation Finished\n";
}

void ProcessSimulator::ApplyStep(const ProcessStep& step)
{
    switch (step.type)
    {
    case ProcessStepType::Photolithography:
        std::cout << "Photolithography - coat "
                  << step.depth
                  << " nm photoresist, expose and develop mask window\n";

        photolithography_.PatternBoxRegion(
            mesh_,
            step.iStart,
            step.iEnd,
            step.jStart,
            step.jEnd,
            step.depth
        );
        break;

    case ProcessStepType::Etch:
        std::cout << "Etch region by "
                  << step.depth
                  << " nm (photoresist-masked)\n";

        etcher_.EtchBoxRegion(
            mesh_,
            step.iStart,
            step.iEnd,
            step.jStart,
            step.jEnd,
            step.depth
        );
        break;

    case ProcessStepType::Deposit:
        std::cout << "Deposit "
                  << MaterialTypeToString(step.material)
                  << " by "
                  << step.depth
                  << " nm into the region\n";

        depositor_.DepositBoxRegion(
            mesh_,
            step.iStart,
            step.iEnd,
            step.jStart,
            step.jEnd,
            step.depth,
            step.material
        );
        break;

    case ProcessStepType::StripResist:
    {
        const double removedVolume = photolithography_.StripResist(mesh_);

        std::cout << "Strip remaining photoresist (ashing) - removed "
                  << removedVolume
                  << " nm^3\n";
        break;
    }
    }
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
