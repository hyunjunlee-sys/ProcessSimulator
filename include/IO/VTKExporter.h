#ifndef VTKEXPORTER_H
#define VTKEXPORTER_H

#include "Mesh/MeshGrid.h"

#include <string>

// Writes the mesh as a legacy VTK structured points file
// (ASCII, cell data) that can be opened directly in ParaView.
class VTKExporter
{
public:
    void ExportMeshGrid(
        const MeshGrid& mesh,
        const std::string& filename
    ) const;

    // Converts material fractions to real heights and writes one
    // hexahedron per material layer (unstructured grid). Partially
    // filled cells appear as shorter boxes, so sub-cell etch and
    // deposition steps are visible as actual topography in ParaView.
    void ExportMaterialGeometry(
        const MeshGrid& mesh,
        const std::string& filename
    ) const;
};

#endif
