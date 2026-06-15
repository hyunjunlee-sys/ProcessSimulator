#include "Process/Depositor.h"
#include "IO/SimulationDefaults.h"
#include "Process/RegionColumnIterator.h"

#include <algorithm>
#include <iostream>

void Depositor::DepositBoxRegion(
    MeshGrid& mesh,
    int iStart,
    int iEnd,
    int jStart,
    int jEnd,
    double depositionDepth,
    MaterialType material
) const
{
    if (depositionDepth <= 0.0 || material == MaterialType::Empty)
    {
        return;
    }

    double maxRemainingDepth = 0.0;

    ForEachColumnInBox(
        mesh,
        iStart,
        iEnd,
        jStart,
        jEnd,
        [&](int i, int j)
        {
            const double remainingDepth =
                DepositColumn(mesh, i, j, depositionDepth, material);

            maxRemainingDepth = std::max(maxRemainingDepth, remainingDepth);
        }
    );

    if (maxRemainingDepth > SimulationDefaults::kDepthWarningEpsilon)
    {
        std::cerr << "Warning: Could not deposit all requested material by "
                  << "up to "
                  << maxRemainingDepth
                  << " nm in the specified region (insufficient void space "
                  << "or a third material would be required in a cell).\n";
    }
}

// Fills void space from the bottom of the column upward. Depth exceeding
// one cell's void space propagates into the cells above. Stops early if a
// cell cannot accept the material (it already holds two other materials),
// since filling above it would leave a buried void. Returns the unused
// deposition depth.
double Depositor::DepositColumn(
    MeshGrid& mesh,
    int i,
    int j,
    double depositionDepth,
    MaterialType material
) const
{
    double remainingDepth = depositionDepth;

    for (int k = 0; k < mesh.GetNz() && remainingDepth > 0.0; ++k)
    {
        MeshCell& cell = mesh.At(i, j, k);

        const double voidDepth =
            (1.0 - cell.GetMaterialFraction()) * cell.GetDz();

        if (voidDepth <= kFractionEpsilon * cell.GetDz())
        {
            continue;
        }

        const double requestedDepth = std::min(remainingDepth, voidDepth);

        const double addedFraction =
            cell.AddFraction(requestedDepth / cell.GetDz(), material);

        const double addedDepth = addedFraction * cell.GetDz();

        if (addedDepth <= kFractionEpsilon * cell.GetDz())
        {
            // The cell has void but rejected the material (two-layer
            // limit). Filling cells above would bury this void, so stop.
            break;
        }

        remainingDepth -= addedDepth;
    }

    return remainingDepth;
}
