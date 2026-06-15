#ifndef PROCESSSTEP_H
#define PROCESSSTEP_H

#include "Materials/Material.h"

enum class ProcessStepType
{
    Photolithography,
    Etch,
    Deposit,
    StripResist
};

// A single ordered process step. The recipe is just a list of these,
// so adding or reordering steps requires no code changes.
struct ProcessStep
{
    ProcessStepType type = ProcessStepType::Etch;

    // Inclusive column box the step acts on. When `hasRegion` is false
    // the reader resolves it to the full wafer during validation.
    int iStart = 0;
    int iEnd = 0;
    int jStart = 0;
    int jEnd = 0;
    bool hasRegion = false;

    // Etch/deposit depth or photoresist thickness, in nm.
    double depth = 0.0;

    // Deposited material (Deposit steps only).
    MaterialType material = MaterialType::Empty;
};

inline const char* ProcessStepTypeToString(ProcessStepType type)
{
    switch (type)
    {
    case ProcessStepType::Photolithography:
        return "Photolithography";
    case ProcessStepType::Etch:
        return "Etch";
    case ProcessStepType::Deposit:
        return "Deposit";
    case ProcessStepType::StripResist:
        return "StripResist";
    default:
        return "Unknown";
    }
}

#endif
