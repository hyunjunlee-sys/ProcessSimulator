#include "IO/InputReader.h"
#include "IO/SimulationDefaults.h"

#include <cstddef>
#include <fstream>
#include <stdexcept>
#include <string>

namespace
{
    std::string Trim(const std::string& text)
    {
        const std::string whitespace = " \t\r\n";

        const std::size_t begin = text.find_first_not_of(whitespace);

        if (begin == std::string::npos)
        {
            return "";
        }

        const std::size_t end = text.find_last_not_of(whitespace);

        return text.substr(begin, end - begin + 1);
    }

    int ParseInt(const std::string& key, const std::string& value)
    {
        try
        {
            return std::stoi(value);
        }
        catch (const std::exception&)
        {
            throw std::runtime_error(
                "Invalid integer value for " + key + ": '" + value + "'"
            );
        }
    }

    double ParseDouble(const std::string& key, const std::string& value)
    {
        try
        {
            return std::stod(value);
        }
        catch (const std::exception&)
        {
            throw std::runtime_error(
                "Invalid numeric value for " + key + ": '" + value + "'"
            );
        }
    }

    MaterialType ParseMaterial(const std::string& key, const std::string& value)
    {
        const MaterialType material = MaterialTypeFromString(value);

        if (material == MaterialType::Empty)
        {
            throw std::runtime_error(
                "Invalid material for " + key + ": '" + value +
                "' (expected Silicon, Oxide, Nitride, Metal, or Photoresist)"
            );
        }

        return material;
    }

    void ValidateConfig(const SimulationConfig& config)
    {
        if (config.nx <= 0 || config.ny <= 0 || config.nz <= 0)
        {
            throw std::runtime_error(
                "Grid size (NX, NY, NZ) must be positive."
            );
        }

        if (config.dx <= 0.0 || config.dy <= 0.0 || config.dz <= 0.0)
        {
            throw std::runtime_error(
                "Cell size (DX, DY, DZ) must be positive."
            );
        }

        if (config.regionIStart > config.regionIEnd ||
            config.regionJStart > config.regionJEnd)
        {
            throw std::runtime_error(
                "Region start index must not exceed end index."
            );
        }

        if (config.regionIStart < 0 ||
            config.regionIEnd >= config.nx ||
            config.regionJStart < 0 ||
            config.regionJEnd >= config.ny)
        {
            throw std::runtime_error(
                "Region indices must be within mesh bounds: "
                "I in [0, " + std::to_string(config.nx - 1) + "], "
                "J in [0, " + std::to_string(config.ny - 1) + "]."
            );
        }

        if (config.substrateLayers < 1 ||
            config.substrateLayers > config.nz)
        {
            throw std::runtime_error(
                "SUBSTRATE_LAYERS must be between 1 and NZ ("
                + std::to_string(config.nz) + ")."
            );
        }

        if (config.etchDepth < 0.0 ||
            config.etchDepth2 < 0.0 ||
            config.depositDepth < 0.0 ||
            config.resistThickness < 0.0)
        {
            throw std::runtime_error(
                "Process depths must not be negative."
            );
        }

        const double headroom =
            (config.nz - config.substrateLayers) * config.dz;

        if (config.resistThickness > headroom)
        {
            throw std::runtime_error(
                "RESIST_THICKNESS (" +
                std::to_string(config.resistThickness) +
                " nm) does not fit above the substrate; available "
                "headroom is " + std::to_string(headroom) + " nm. "
                "Increase NZ or decrease SUBSTRATE_LAYERS."
            );
        }
    }
}

SimulationConfig InputReader::CreateDefaultConfig()
{
    SimulationConfig config{};

    config.nx = SimulationDefaults::kNx;
    config.ny = SimulationDefaults::kNy;
    config.nz = SimulationDefaults::kNz;

    config.substrateLayers = SimulationDefaults::kSubstrateLayers;

    config.dx = SimulationDefaults::kDx;
    config.dy = SimulationDefaults::kDy;
    config.dz = SimulationDefaults::kDz;

    config.regionIStart = SimulationDefaults::kRegionIStart;
    config.regionIEnd = SimulationDefaults::kRegionIEnd;
    config.regionJStart = SimulationDefaults::kRegionJStart;
    config.regionJEnd = SimulationDefaults::kRegionJEnd;

    config.etchDepth = SimulationDefaults::kEtchDepth;
    config.etchDepth2 = SimulationDefaults::kEtchDepth2;
    config.depositDepth = SimulationDefaults::kDepositDepth;
    config.resistThickness = SimulationDefaults::kResistThickness;

    config.initialMaterial = SimulationDefaults::kInitialMaterial;

    return config;
}

SimulationConfig InputReader::Read(const std::string& filename) const
{
    std::ifstream file(filename);

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open input file: " + filename);
    }

    SimulationConfig config = CreateDefaultConfig();

    std::string line;

    while (std::getline(file, line))
    {
        const std::size_t commentPos = line.find('#');

        if (commentPos != std::string::npos)
        {
            line = line.substr(0, commentPos);
        }

        line = Trim(line);

        if (line.empty())
        {
            continue;
        }

        const std::size_t separatorPos = line.find('=');

        if (separatorPos == std::string::npos)
        {
            throw std::runtime_error(
                "Invalid input line (expected KEY=VALUE): '" + line + "'"
            );
        }

        const std::string key = Trim(line.substr(0, separatorPos));
        const std::string value = Trim(line.substr(separatorPos + 1));

        if (key == "NX")
        {
            config.nx = ParseInt(key, value);
        }
        else if (key == "NY")
        {
            config.ny = ParseInt(key, value);
        }
        else if (key == "NZ")
        {
            config.nz = ParseInt(key, value);
        }
        else if (key == "SUBSTRATE_LAYERS")
        {
            config.substrateLayers = ParseInt(key, value);
        }
        else if (key == "DX")
        {
            config.dx = ParseDouble(key, value);
        }
        else if (key == "DY")
        {
            config.dy = ParseDouble(key, value);
        }
        else if (key == "DZ")
        {
            config.dz = ParseDouble(key, value);
        }
        else if (key == "REGION_I_START")
        {
            config.regionIStart = ParseInt(key, value);
        }
        else if (key == "REGION_I_END")
        {
            config.regionIEnd = ParseInt(key, value);
        }
        else if (key == "REGION_J_START")
        {
            config.regionJStart = ParseInt(key, value);
        }
        else if (key == "REGION_J_END")
        {
            config.regionJEnd = ParseInt(key, value);
        }
        else if (key == "ETCH_DEPTH")
        {
            config.etchDepth = ParseDouble(key, value);
        }
        else if (key == "ETCH_DEPTH_2")
        {
            config.etchDepth2 = ParseDouble(key, value);
        }
        else if (key == "DEPOSIT_DEPTH")
        {
            config.depositDepth = ParseDouble(key, value);
        }
        else if (key == "RESIST_THICKNESS")
        {
            config.resistThickness = ParseDouble(key, value);
        }
        else if (key == "INITIAL_MATERIAL")
        {
            config.initialMaterial = ParseMaterial(key, value);
        }
        else
        {
            throw std::runtime_error("Unknown input key: '" + key + "'");
        }
    }

    ValidateConfig(config);

    return config;
}
