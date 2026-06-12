#ifndef ETCHER_H
#define ETCHER_H

#include "Mesh/MeshGrid.h"

class Etcher
{
public:
    void EtchBoxRegion(
        MeshGrid& mesh,
        int iStart,
        int iEnd,
        int jStart,
        int jEnd,
        double etchDepth
    ) const;

private:
    double EtchColumn(
        MeshGrid& mesh,
        int i,
        int j,
        double etchDepth
    ) const;
};

#endif