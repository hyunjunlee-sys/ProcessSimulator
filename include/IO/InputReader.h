#ifndef INPUTREADER_H
#define INPUTREADER_H

#include "Materials/Material.h"

#include <string>

struct SimulationConfig
{
    int nx;
    int ny;
    int nz;

    int substrateLayers;

    double dx;
    double dy;
    double dz;

    int regionIStart;
    int regionIEnd;
    int regionJStart;
    int regionJEnd;

    double etchDepth;
    double etchDepth2;
    double depositDepth;
    double resistThickness;

    MaterialType initialMaterial;
};

class InputReader
{
public:
    SimulationConfig Read(const std::string& filename) const;

private:
    static SimulationConfig CreateDefaultConfig();
};

#endif
