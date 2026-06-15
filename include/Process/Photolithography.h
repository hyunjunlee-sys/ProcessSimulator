#ifndef PHOTOLITHOGRAPHY_H
#define PHOTOLITHOGRAPHY_H

#include "Mesh/MeshGrid.h"

// Simplified photolithography for the column-based process model:
// 1. Spin-coat photoresist over the whole wafer surface.
// 2. Expose through a box-shaped mask window and develop, which
//    clears the resist inside the window (positive resist model).
// The remaining resist can later be removed with StripResist (ashing).
class Photolithography
{
public:
    void PatternBoxRegion(
        MeshGrid& mesh,
        int iStart,
        int iEnd,
        int jStart,
        int jEnd,
        double resistThickness
    ) const;

    // Removes all photoresist from the wafer and returns the total
    // stripped resist volume in nm^3.
    double StripResist(MeshGrid& mesh) const;

private:
    void CoatResist(MeshGrid& mesh, double resistThickness) const;

    double RemoveResistColumn(MeshGrid& mesh, int i, int j) const;
};

#endif
