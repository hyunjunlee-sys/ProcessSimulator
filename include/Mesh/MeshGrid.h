#ifndef MESHGRID_H
#define MESHGRID_H

#include "Mesh/MeshCell.h"

#include <vector>

class MeshGrid
{
private:
    int nx_;
    int ny_;
    int nz_;

    double dx_;
    double dy_;
    double dz_;

    std::vector<MeshCell> cells_;

private:
    int GetIndex(int i, int j, int k) const;

public:
    // The bottom substrateLayers z-layers are filled with
    // initialMaterial; the layers above start empty.
    MeshGrid(
        int nx,
        int ny,
        int nz,
        double dx,
        double dy,
        double dz,
        MaterialType initialMaterial,
        int substrateLayers
    );

    int GetNx() const;
    int GetNy() const;
    int GetNz() const;

    double GetDx() const;
    double GetDy() const;
    double GetDz() const;

    int GetTotalCellCount() const;

    bool IsValidIndex(int i, int j, int k) const;

    MeshCell& At(int i, int j, int k);
    const MeshCell& At(int i, int j, int k) const;

    const std::vector<MeshCell>& GetCells() const;

    int CountCellsByMaterial(MaterialType material) const;
    double CalculateTotalRemainingVolume(MaterialType material) const;

    void PrintSummary() const;
    void PrintSliceZ(int k) const;
    void PrintAllSlicesZ() const;
};

#endif
