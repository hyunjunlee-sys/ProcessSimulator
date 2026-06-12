#ifndef CSVEXPORTER_H
#define CSVEXPORTER_H

#include "Mesh/MeshGrid.h"

#include <string>

class CSVExporter
{
public:
    void ExportMeshGrid(
        const MeshGrid& mesh,
        const std::string& filename
    ) const;
};

#endif