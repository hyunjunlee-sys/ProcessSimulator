#include "Mesh/MeshGrid.h"

#include <cctype>
#include <cstddef>
#include <iostream>
#include <stdexcept>

namespace
{
    bool CellContainsMaterial(const MeshCell& cell, MaterialType material)
    {
        return cell.GetRemainingVolume(material) >
               kFractionEpsilon * cell.GetCellVolume();
    }

    char GetSliceSymbol(const MeshCell& cell)
    {
        if (cell.IsEmpty())
        {
            return '.';
        }

        const MaterialType surfaceMaterial = cell.GetSurfaceMaterial();
        const char symbol = MaterialTypeToSymbol(surfaceMaterial);

        if (cell.IsFull() && !cell.HasMultipleMaterials())
        {
            return symbol;
        }

        return static_cast<char>(std::tolower(symbol));
    }
}

MeshGrid::MeshGrid(
    int nx,
    int ny,
    int nz,
    double dx,
    double dy,
    double dz,
    MaterialType initialMaterial,
    int substrateLayers
)
    : nx_(nx),
      ny_(ny),
      nz_(nz),
      dx_(dx),
      dy_(dy),
      dz_(dz)
{
    if (nx_ <= 0 || ny_ <= 0 || nz_ <= 0)
    {
        throw std::invalid_argument("Mesh grid size must be positive.");
    }

    if (dx_ <= 0.0 || dy_ <= 0.0 || dz_ <= 0.0)
    {
        throw std::invalid_argument("Mesh cell size must be positive.");
    }

    if (substrateLayers < 0 || substrateLayers > nz_)
    {
        throw std::invalid_argument(
            "Substrate layer count must be between 0 and NZ."
        );
    }

    cells_.reserve(
        static_cast<std::size_t>(nx_) *
        static_cast<std::size_t>(ny_) *
        static_cast<std::size_t>(nz_)
    );

    for (int k = 0; k < nz_; ++k)
    {
        for (int j = 0; j < ny_; ++j)
        {
            for (int i = 0; i < nx_; ++i)
            {
                double x = i * dx_;
                double y = j * dy_;
                double z = k * dz_;

                cells_.emplace_back(
                    i,
                    j,
                    k,
                    x,
                    y,
                    z,
                    dx_,
                    dy_,
                    dz_,
                    k < substrateLayers ? initialMaterial
                                        : MaterialType::Empty
                );
            }
        }
    }
}

int MeshGrid::GetIndex(int i, int j, int k) const
{
    return k * nx_ * ny_ + j * nx_ + i;
}

int MeshGrid::GetNx() const
{
    return nx_;
}

int MeshGrid::GetNy() const
{
    return ny_;
}

int MeshGrid::GetNz() const
{
    return nz_;
}

double MeshGrid::GetDx() const
{
    return dx_;
}

double MeshGrid::GetDy() const
{
    return dy_;
}

double MeshGrid::GetDz() const
{
    return dz_;
}

int MeshGrid::GetTotalCellCount() const
{
    return static_cast<int>(cells_.size());
}

bool MeshGrid::IsValidIndex(int i, int j, int k) const
{
    return i >= 0 && i < nx_ &&
           j >= 0 && j < ny_ &&
           k >= 0 && k < nz_;
}

MeshCell& MeshGrid::At(int i, int j, int k)
{
    if (!IsValidIndex(i, j, k))
    {
        throw std::out_of_range("Invalid mesh index.");
    }

    return cells_[static_cast<std::size_t>(GetIndex(i, j, k))];
}

const MeshCell& MeshGrid::At(int i, int j, int k) const
{
    if (!IsValidIndex(i, j, k))
    {
        throw std::out_of_range("Invalid mesh index.");
    }

    return cells_[static_cast<std::size_t>(GetIndex(i, j, k))];
}

const std::vector<MeshCell>& MeshGrid::GetCells() const
{
    return cells_;
}

int MeshGrid::CountCellsByMaterial(MaterialType material) const
{
    int count = 0;

    for (const MeshCell& cell : cells_)
    {
        if (material == MaterialType::Empty)
        {
            if (cell.IsEmpty())
            {
                ++count;
            }
        }
        else if (CellContainsMaterial(cell, material))
        {
            ++count;
        }
    }

    return count;
}

double MeshGrid::CalculateTotalRemainingVolume(MaterialType material) const
{
    double totalVolume = 0.0;

    for (const MeshCell& cell : cells_)
    {
        totalVolume += cell.GetRemainingVolume(material);
    }

    return totalVolume;
}

void MeshGrid::PrintSummary() const
{
    std::cout << "\n========== Mesh Summary ==========\n";

    std::cout << "Grid size: "
              << nx_ << " x "
              << ny_ << " x "
              << nz_ << "\n";

    std::cout << "Cell size: "
              << dx_ << " nm x "
              << dy_ << " nm x "
              << dz_ << " nm\n";

    std::cout << "Total cells: "
              << GetTotalCellCount() << "\n";

    std::cout << "Silicon cells: "
              << CountCellsByMaterial(MaterialType::Silicon) << "\n";

    std::cout << "Oxide cells: "
              << CountCellsByMaterial(MaterialType::Oxide) << "\n";

    std::cout << "Nitride cells: "
              << CountCellsByMaterial(MaterialType::Nitride) << "\n";

    std::cout << "Metal cells: "
              << CountCellsByMaterial(MaterialType::Metal) << "\n";

    std::cout << "Photoresist cells: "
              << CountCellsByMaterial(MaterialType::Photoresist) << "\n";

    std::cout << "Empty cells: "
              << CountCellsByMaterial(MaterialType::Empty) << "\n";

    std::cout << "Remaining Silicon volume: "
              << CalculateTotalRemainingVolume(MaterialType::Silicon)
              << " nm^3\n";

    std::cout << "Remaining Oxide volume: "
              << CalculateTotalRemainingVolume(MaterialType::Oxide)
              << " nm^3\n";

    std::cout << "==================================\n";
}

void MeshGrid::PrintSliceZ(int k) const
{
    if (k < 0 || k >= nz_)
    {
        throw std::out_of_range("Invalid Z slice index.");
    }

    std::cout << "\n========== Z Slice k = " << k << " ==========\n";

    for (int j = ny_ - 1; j >= 0; --j)
    {
        for (int i = 0; i < nx_; ++i)
        {
            const MeshCell& cell = At(i, j, k);
            std::cout << GetSliceSymbol(cell) << " ";
        }

        std::cout << "\n";
    }

    std::cout << "Legend: S=Si O=Oxide N=Nitride M=Metal P=Photoresist "
              << "(uppercase full, lowercase partial/mixed), . empty\n";
}

void MeshGrid::PrintAllSlicesZ() const
{
    for (int k = nz_ - 1; k >= 0; --k)
    {
        PrintSliceZ(k);
    }
}
