#ifndef INPUTREADER_H
#define INPUTREADER_H

#include "Materials/Material.h"
#include "Process/ProcessStep.h"

#include <string>
#include <vector>

struct SimulationConfig
{
    int nx;
    int ny;
    int nz;

    int substrateLayers;

    double dx;
    double dy;
    double dz;

    MaterialType initialMaterial;

    bool verbose;

    // Ordered process recipe executed by the simulator.
    std::vector<ProcessStep> steps;
};

class InputReader
{
public:
    SimulationConfig Read(const std::string& filename) const;

private:
    static SimulationConfig CreateDefaultConfig();
};

#endif
