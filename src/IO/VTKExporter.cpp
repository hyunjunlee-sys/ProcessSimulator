#include "IO/VTKExporter.h"

#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <vector>

namespace
{
    // One axis-aligned box representing a single material layer
    // inside a mesh cell, with its real (fraction-scaled) height.
    struct MaterialBox
    {
        double x;
        double y;
        double zBottom;
        double dx;
        double dy;
        double height;
        int materialId;
        double fraction;
    };

    // Material IDs in the VTK file:
    // 0=Empty, 1=Silicon, 2=Oxide, 3=Nitride, 4=Metal, 5=Photoresist
    int MaterialTypeToId(MaterialType material)
    {
        return static_cast<int>(material);
    }

    template <typename Getter>
    void WriteIntScalars(
        std::ofstream& file,
        const MeshGrid& mesh,
        const std::string& name,
        Getter getter
    )
    {
        file << "SCALARS " << name << " int 1\n"
             << "LOOKUP_TABLE default\n";

        for (const MeshCell& cell : mesh.GetCells())
        {
            file << getter(cell) << "\n";
        }
    }

    template <typename Getter>
    void WriteDoubleScalars(
        std::ofstream& file,
        const MeshGrid& mesh,
        const std::string& name,
        Getter getter
    )
    {
        file << "SCALARS " << name << " double 1\n"
             << "LOOKUP_TABLE default\n";

        for (const MeshCell& cell : mesh.GetCells())
        {
            file << getter(cell) << "\n";
        }
    }
}

void VTKExporter::ExportMeshGrid(
    const MeshGrid& mesh,
    const std::string& filename
) const
{
    std::ofstream file(filename);

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open VTK output file: " + filename);
    }

    // Cells are stored with i varying fastest, then j, then k,
    // which matches the VTK structured points cell ordering.
    file << "# vtk DataFile Version 3.0\n"
         << "ProcessSimulator result "
         << "(materials: 0=Empty 1=Silicon 2=Oxide 3=Nitride 4=Metal "
         << "5=Photoresist)\n"
         << "ASCII\n"
         << "DATASET STRUCTURED_POINTS\n"
         << "DIMENSIONS "
         << mesh.GetNx() + 1 << " "
         << mesh.GetNy() + 1 << " "
         << mesh.GetNz() + 1 << "\n"
         << "ORIGIN 0 0 0\n"
         << "SPACING "
         << mesh.GetDx() << " "
         << mesh.GetDy() << " "
         << mesh.GetDz() << "\n"
         << "CELL_DATA " << mesh.GetTotalCellCount() << "\n";

    file << std::fixed << std::setprecision(6);

    WriteIntScalars(
        file,
        mesh,
        "surface_material",
        [](const MeshCell& cell)
        {
            return MaterialTypeToId(cell.GetSurfaceMaterial());
        }
    );

    WriteIntScalars(
        file,
        mesh,
        "primary_material",
        [](const MeshCell& cell)
        {
            return MaterialTypeToId(cell.GetPrimaryMaterial());
        }
    );

    WriteDoubleScalars(
        file,
        mesh,
        "primary_fraction",
        [](const MeshCell& cell)
        {
            return cell.GetPrimaryFraction();
        }
    );

    WriteIntScalars(
        file,
        mesh,
        "secondary_material",
        [](const MeshCell& cell)
        {
            return MaterialTypeToId(cell.GetSecondaryMaterial());
        }
    );

    WriteDoubleScalars(
        file,
        mesh,
        "secondary_fraction",
        [](const MeshCell& cell)
        {
            return cell.GetSecondaryFraction();
        }
    );

    WriteDoubleScalars(
        file,
        mesh,
        "total_fraction",
        [](const MeshCell& cell)
        {
            return cell.GetMaterialFraction();
        }
    );
}

void VTKExporter::ExportMaterialGeometry(
    const MeshGrid& mesh,
    const std::string& filename
) const
{
    // Materials fill each cell from its bottom: the primary layer
    // sits on the cell floor and the secondary layer stacks on top,
    // matching how the etcher removes from the top and the
    // depositor fills from the bottom.
    std::vector<MaterialBox> boxes;

    for (const MeshCell& cell : mesh.GetCells())
    {
        double layerBottom = cell.GetZ();

        const MaterialType primaryMaterial = cell.GetPrimaryMaterial();
        const double primaryFraction = cell.GetPrimaryFraction();

        if (primaryMaterial != MaterialType::Empty &&
            primaryFraction > kFractionEpsilon)
        {
            const double height = primaryFraction * cell.GetDz();

            boxes.push_back({
                cell.GetX(),
                cell.GetY(),
                layerBottom,
                cell.GetDx(),
                cell.GetDy(),
                height,
                static_cast<int>(primaryMaterial),
                primaryFraction
            });

            layerBottom += height;
        }

        const MaterialType secondaryMaterial = cell.GetSecondaryMaterial();
        const double secondaryFraction = cell.GetSecondaryFraction();

        if (secondaryMaterial != MaterialType::Empty &&
            secondaryFraction > kFractionEpsilon)
        {
            const double height = secondaryFraction * cell.GetDz();

            boxes.push_back({
                cell.GetX(),
                cell.GetY(),
                layerBottom,
                cell.GetDx(),
                cell.GetDy(),
                height,
                static_cast<int>(secondaryMaterial),
                secondaryFraction
            });
        }
    }

    std::ofstream file(filename);

    if (!file.is_open())
    {
        throw std::runtime_error(
            "Failed to open VTK geometry output file: " + filename
        );
    }

    const std::size_t boxCount = boxes.size();
    const std::size_t pointCount = boxCount * 8;

    file << "# vtk DataFile Version 3.0\n"
         << "ProcessSimulator material geometry "
         << "(materials: 0=Empty 1=Silicon 2=Oxide 3=Nitride 4=Metal "
         << "5=Photoresist)\n"
         << "ASCII\n"
         << "DATASET UNSTRUCTURED_GRID\n"
         << "POINTS " << pointCount << " double\n";

    file << std::fixed << std::setprecision(6);

    for (const MaterialBox& box : boxes)
    {
        const double x0 = box.x;
        const double x1 = box.x + box.dx;
        const double y0 = box.y;
        const double y1 = box.y + box.dy;
        const double z0 = box.zBottom;
        const double z1 = box.zBottom + box.height;

        // VTK_HEXAHEDRON ordering: bottom face CCW, then top face CCW.
        file << x0 << " " << y0 << " " << z0 << "\n"
             << x1 << " " << y0 << " " << z0 << "\n"
             << x1 << " " << y1 << " " << z0 << "\n"
             << x0 << " " << y1 << " " << z0 << "\n"
             << x0 << " " << y0 << " " << z1 << "\n"
             << x1 << " " << y0 << " " << z1 << "\n"
             << x1 << " " << y1 << " " << z1 << "\n"
             << x0 << " " << y1 << " " << z1 << "\n";
    }

    file << "CELLS " << boxCount << " " << boxCount * 9 << "\n";

    for (std::size_t boxIndex = 0; boxIndex < boxCount; ++boxIndex)
    {
        const std::size_t base = boxIndex * 8;

        file << "8";

        for (std::size_t corner = 0; corner < 8; ++corner)
        {
            file << " " << base + corner;
        }

        file << "\n";
    }

    file << "CELL_TYPES " << boxCount << "\n";

    for (std::size_t boxIndex = 0; boxIndex < boxCount; ++boxIndex)
    {
        file << "12\n";
    }

    file << "CELL_DATA " << boxCount << "\n"
         << "SCALARS material int 1\n"
         << "LOOKUP_TABLE default\n";

    for (const MaterialBox& box : boxes)
    {
        file << box.materialId << "\n";
    }

    file << "SCALARS fraction double 1\n"
         << "LOOKUP_TABLE default\n";

    for (const MaterialBox& box : boxes)
    {
        file << box.fraction << "\n";
    }
}
