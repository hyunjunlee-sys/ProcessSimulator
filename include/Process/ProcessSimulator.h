#ifndef PROCESSSIMULATOR_H
#define PROCESSSIMULATOR_H

#include "Mesh/MeshGrid.h"
#include "Process/Etcher.h"
#include "Process/Depositor.h"
#include "Process/Photolithography.h"
#include "IO/CSVExporter.h"
#include "IO/VTKExporter.h"
#include "IO/InputReader.h"

#include <string>

class ProcessSimulator
{
private:
    MeshGrid mesh_;
    Etcher etcher_;
    Depositor depositor_;
    Photolithography photolithography_;
    CSVExporter csvExporter_;
    VTKExporter vtkExporter_;

public:
    ProcessSimulator(
        int nx,
        int ny,
        int nz,
        double dx,
        double dy,
        double dz,
        MaterialType initialMaterial,
        int substrateLayers
    );

    void RunSimpleProcess(const SimulationConfig& config);
    void ExportResult(const std::string& filename) const;
};

#endif
