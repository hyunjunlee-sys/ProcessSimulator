#include "Process/Etcher.h"
#include "IO/SimulationDefaults.h"
#include "Process/RegionColumnIterator.h"

#include <algorithm>
#include <iostream>

void Etcher::EtchBoxRegion(
    MeshGrid& mesh,
    int iStart,
    int iEnd,
    int jStart,
    int jEnd,
    double etchDepth
) const
{
    if (etchDepth <= 0.0)
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
            const double remainingDepth = EtchColumn(mesh, i, j, etchDepth);
            maxRemainingDepth = std::max(maxRemainingDepth, remainingDepth);
        }
    );

    if (maxRemainingDepth > SimulationDefaults::kDepthWarningEpsilon)
    {
        std::cerr << "Warning: Etch depth exceeded available material by up to "
                  << maxRemainingDepth
                  << " nm in the specified region.\n";
    }
}

// Removes material from the top of the column downward. Depth exceeding
// one cell propagates into the cells below. Returns unused etch depth.
double Etcher::EtchColumn(
    MeshGrid& mesh,
    int i,
    int j,
    double etchDepth
) const
{
    double remainingDepth = etchDepth;

    for (int k = mesh.GetNz() - 1; k >= 0 && remainingDepth > 0.0; --k)
    {
        MeshCell& cell = mesh.At(i, j, k);

        if (cell.IsEmpty())
        {
            continue;
        }

        const double materialDepth =
            cell.GetMaterialFraction() * cell.GetDz();

        const double removedDepth = std::min(remainingDepth, materialDepth);

        cell.RemoveFraction(removedDepth / cell.GetDz());
        remainingDepth -= removedDepth;
    }

    return remainingDepth;
}
