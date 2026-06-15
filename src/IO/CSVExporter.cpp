#include "IO/CSVExporter.h"

#include <fstream>
#include <iomanip>
#include <stdexcept>

void CSVExporter::ExportMeshGrid(
    const MeshGrid& mesh,
    const std::string& filename
) const
{
    std::ofstream file(filename);

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open CSV output file: " + filename);
    }

    file << "i,j,k,"
         << "x,y,z,"
         << "dx,dy,dz,"
         << "primary_material,"
         << "primary_fraction,"
         << "secondary_material,"
         << "secondary_fraction,"
         << "surface_material,"
         << "total_fraction,"
         << "cell_volume,"
         << "remaining_volume\n";

    file << std::fixed << std::setprecision(6);

    for (const MeshCell& cell : mesh.GetCells())
    {
        file << cell.GetI() << ","
             << cell.GetJ() << ","
             << cell.GetK() << ","
             << cell.GetX() << ","
             << cell.GetY() << ","
             << cell.GetZ() << ","
             << cell.GetDx() << ","
             << cell.GetDy() << ","
             << cell.GetDz() << ","
             << MaterialTypeToString(cell.GetPrimaryMaterial()) << ","
             << cell.GetPrimaryFraction() << ","
             << MaterialTypeToString(cell.GetSecondaryMaterial()) << ","
             << cell.GetSecondaryFraction() << ","
             << MaterialTypeToString(cell.GetSurfaceMaterial()) << ","
             << cell.GetMaterialFraction() << ","
             << cell.GetCellVolume() << ","
             << cell.GetRemainingVolume()
             << "\n";
    }
}
