#ifndef MATERIAL_H
#define MATERIAL_H

#include <algorithm>
#include <cctype>
#include <optional>
#include <string>

enum class MaterialType
{
    Empty,
    Silicon,
    Oxide,
    Nitride,
    Metal,
    Photoresist
};

inline std::string MaterialTypeToString(MaterialType material)
{
    switch (material)
    {
    case MaterialType::Empty:
        return "Empty";
    case MaterialType::Silicon:
        return "Silicon";
    case MaterialType::Oxide:
        return "Oxide";
    case MaterialType::Nitride:
        return "Nitride";
    case MaterialType::Metal:
        return "Metal";
    case MaterialType::Photoresist:
        return "Photoresist";
    default:
        return "Unknown";
    }
}

inline char MaterialTypeToSymbol(MaterialType material)
{
    switch (material)
    {
    case MaterialType::Empty:
        return '.';
    case MaterialType::Silicon:
        return 'S';
    case MaterialType::Oxide:
        return 'O';
    case MaterialType::Nitride:
        return 'N';
    case MaterialType::Metal:
        return 'M';
    case MaterialType::Photoresist:
        return 'P';
    default:
        return '?';
    }
}

// Returns std::nullopt for an unrecognized name. The literal "empty"
// maps to MaterialType::Empty, so callers can distinguish "no such
// material" (nullopt) from the intentional empty material.
inline std::optional<MaterialType> MaterialTypeFromString(const std::string& name)
{
    std::string normalized = name;
    std::transform(
        normalized.begin(),
        normalized.end(),
        normalized.begin(),
        [](unsigned char character)
        {
            return static_cast<char>(std::tolower(character));
        }
    );

    if (normalized == "empty")
    {
        return MaterialType::Empty;
    }

    if (normalized == "silicon" || normalized == "si")
    {
        return MaterialType::Silicon;
    }

    if (normalized == "oxide" || normalized == "sio2")
    {
        return MaterialType::Oxide;
    }

    if (normalized == "nitride" || normalized == "si3n4")
    {
        return MaterialType::Nitride;
    }

    if (normalized == "metal")
    {
        return MaterialType::Metal;
    }

    if (normalized == "photoresist" || normalized == "resist" ||
        normalized == "pr")
    {
        return MaterialType::Photoresist;
    }

    return std::nullopt;
}

#endif