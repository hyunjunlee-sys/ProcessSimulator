#include "Process/Photolithography.h"
#include "Process/Depositor.h"
#include "Process/RegionColumnIterator.h"

void Photolithography::PatternBoxRegion(
    MeshGrid& mesh,
    int iStart,
    int iEnd,
    int jStart,
    int jEnd,
    double resistThickness
) const
{
    if (resistThickness <= 0.0)
    {
        return;
    }

    CoatResist(mesh, resistThickness);

    // Expose & develop: the mask window is cleared of resist.
    ForEachColumnInBox(
        mesh,
        iStart,
        iEnd,
        jStart,
        jEnd,
        [&](int i, int j)
        {
            RemoveResistColumn(mesh, i, j);
        }
    );
}

void Photolithography::StripResist(MeshGrid& mesh) const
{
    ForEachColumnInBox(
        mesh,
        0,
        mesh.GetNx() - 1,
        0,
        mesh.GetNy() - 1,
        [&](int i, int j)
        {
            RemoveResistColumn(mesh, i, j);
        }
    );
}

// Blanket coat: fills the available space on top of every column,
// which is how spin coating planarizes the wafer surface.
void Photolithography::CoatResist(
    MeshGrid& mesh,
    double resistThickness
) const
{
    const Depositor depositor{};

    depositor.DepositBoxRegion(
        mesh,
        0,
        mesh.GetNx() - 1,
        0,
        mesh.GetNy() - 1,
        resistThickness,
        MaterialType::Photoresist
    );
}

// Returns the removed resist depth in nm.
double Photolithography::RemoveResistColumn(
    MeshGrid& mesh,
    int i,
    int j
) const
{
    double removedDepth = 0.0;

    for (int k = mesh.GetNz() - 1; k >= 0; --k)
    {
        MeshCell& cell = mesh.At(i, j, k);

        removedDepth +=
            cell.RemoveMaterial(MaterialType::Photoresist) * cell.GetDz();
    }

    return removedDepth;
}
