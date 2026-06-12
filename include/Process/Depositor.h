#ifndef DEPOSITOR_H
#define DEPOSITOR_H

#include "Mesh/MeshGrid.h"

class Depositor
{
public:
    void DepositBoxRegion(
        MeshGrid& mesh,
        int iStart,
        int iEnd,
        int jStart,
        int jEnd,
        double depositionDepth,
        MaterialType material
    ) const;

private:
    double DepositColumn(
        MeshGrid& mesh,
        int i,
        int j,
        double depositionDepth,
        MaterialType material
    ) const;
};

#endif
