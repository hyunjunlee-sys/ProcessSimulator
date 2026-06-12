#ifndef SIMULATIONDEFAULTS_H
#define SIMULATIONDEFAULTS_H

#include "Materials/Material.h"

// Single source of truth for simulation defaults (used when input keys are omitted).
namespace SimulationDefaults
{
    inline constexpr int kNx = 10;
    inline constexpr int kNy = 10;
    inline constexpr int kNz = 7;

    // Number of bottom z-layers initially filled with the substrate
    // material. Layers above stay empty so photoresist can be coated.
    inline constexpr int kSubstrateLayers = 5;

    inline constexpr double kDx = 5.0;
    inline constexpr double kDy = 5.0;
    inline constexpr double kDz = 5.0;

    inline constexpr int kRegionIStart = 3;
    inline constexpr int kRegionIEnd = 6;
    inline constexpr int kRegionJStart = 3;
    inline constexpr int kRegionJEnd = 6;

    inline constexpr double kEtchDepth = 2.0;
    inline constexpr double kEtchDepth2 = 3.0;
    inline constexpr double kDepositDepth = 2.0;

    // Photoresist thickness (nm). Zero disables the lithography step.
    inline constexpr double kResistThickness = 5.0;

    inline constexpr MaterialType kInitialMaterial = MaterialType::Silicon;

    // Depths below this threshold (nm) are treated as fully consumed.
    inline constexpr double kDepthWarningEpsilon = 1e-6;
}

#endif
