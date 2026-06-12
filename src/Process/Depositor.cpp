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
        std::cerr << "Warning: Deposition depth exceeded available void space "
                  << "by up to "
                  << maxRemainingDepth
                  << " nm in the specified region.\n";
    }
}

// Fills void space from the bottom of the column upward. Depth exceeding
// one cell's void space propagates into the cells above.
// Returns unused deposition depth.
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

        const double addedDepth = std::min(remainingDepth, voidDepth);

        cell.AddFraction(addedDepth / cell.GetDz(), material);
        remainingDepth -= addedDepth;
    }

    return remainingDepth;
}
