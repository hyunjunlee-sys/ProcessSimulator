#include "IO/InputReader.h"
#include "IO/SimulationDefaults.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <fstream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

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

    std::string ToLower(std::string text)
    {
        std::transform(
            text.begin(),
            text.end(),
            text.begin(),
            [](unsigned char character)
            {
                return static_cast<char>(std::tolower(character));
            }
        );

        return text;
    }

    std::vector<std::string> SplitWhitespace(const std::string& text)
    {
        std::vector<std::string> tokens;
        std::istringstream stream(text);
        std::string token;

        while (stream >> token)
        {
            tokens.push_back(token);
        }

        return tokens;
    }

    std::vector<std::string> SplitByComma(const std::string& text)
    {
        std::vector<std::string> parts;
        std::stringstream stream(text);
        std::string part;

        while (std::getline(stream, part, ','))
        {
            parts.push_back(Trim(part));
        }

        return parts;
    }

    int ParseInt(const std::string& key, const std::string& value)
    {
        try
        {
            std::size_t consumed = 0;
            const int parsed = std::stoi(value, &consumed);

            if (consumed != value.size())
            {
                throw std::invalid_argument("trailing characters");
            }

            return parsed;
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
            std::size_t consumed = 0;
            const double parsed = std::stod(value, &consumed);

            if (consumed != value.size())
            {
                throw std::invalid_argument("trailing characters");
            }

            return parsed;
        }
        catch (const std::exception&)
        {
            throw std::runtime_error(
                "Invalid numeric value for " + key + ": '" + value + "'"
            );
        }
    }

    bool ParseBool(const std::string& key, const std::string& value)
    {
        const std::string normalized = ToLower(value);

        if (normalized == "true" || normalized == "1" ||
            normalized == "yes" || normalized == "on")
        {
            return true;
        }

        if (normalized == "false" || normalized == "0" ||
            normalized == "no" || normalized == "off")
        {
            return false;
        }

        throw std::runtime_error(
            "Invalid boolean value for " + key + ": '" + value +
            "' (expected true/false)"
        );
    }

    MaterialType ParseMaterial(const std::string& key, const std::string& value)
    {
        const std::optional<MaterialType> material =
            MaterialTypeFromString(value);

        if (!material || *material == MaterialType::Empty)
        {
            throw std::runtime_error(
                "Invalid material for " + key + ": '" + value +
                "' (expected Silicon, Oxide, Nitride, Metal, or Photoresist)"
            );
        }

        return *material;
    }

    ProcessStepType ParseStepType(const std::string& token)
    {
        const std::string type = ToLower(token);

        if (type == "litho" || type == "lithography" ||
            type == "photo" || type == "photolithography")
        {
            return ProcessStepType::Photolithography;
        }

        if (type == "etch" || type == "etching")
        {
            return ProcessStepType::Etch;
        }

        if (type == "deposit" || type == "deposition")
        {
            return ProcessStepType::Deposit;
        }

        if (type == "strip" || type == "strip_resist" ||
            type == "ash" || type == "ashing")
        {
            return ProcessStepType::StripResist;
        }

        throw std::runtime_error(
            "Unknown process step type: '" + token +
            "' (expected LITHO, ETCH, DEPOSIT, or STRIP)"
        );
    }

    void ParseRegion(ProcessStep& step, const std::string& value)
    {
        const std::vector<std::string> parts = SplitByComma(value);

        if (parts.size() != 4)
        {
            throw std::runtime_error(
                "STEP region must have 4 comma-separated indices "
                "(iStart,iEnd,jStart,jEnd), got: '" + value + "'"
            );
        }

        step.iStart = ParseInt("region.iStart", parts[0]);
        step.iEnd = ParseInt("region.iEnd", parts[1]);
        step.jStart = ParseInt("region.jStart", parts[2]);
        step.jEnd = ParseInt("region.jEnd", parts[3]);
        step.hasRegion = true;
    }

    ProcessStep ParseStep(const std::string& value)
    {
        const std::vector<std::string> tokens = SplitWhitespace(value);

        if (tokens.empty())
        {
            throw std::runtime_error("STEP requires a type (e.g. ETCH).");
        }

        ProcessStep step;
        step.type = ParseStepType(tokens[0]);

        for (std::size_t index = 1; index < tokens.size(); ++index)
        {
            const std::string& token = tokens[index];
            const std::size_t separatorPos = token.find('=');

            if (separatorPos == std::string::npos)
            {
                throw std::runtime_error(
                    "Invalid STEP parameter (expected key=value): '" +
                    token + "'"
                );
            }

            const std::string parameterKey =
                ToLower(Trim(token.substr(0, separatorPos)));
            const std::string parameterValue =
                Trim(token.substr(separatorPos + 1));

            if (parameterKey == "region")
            {
                ParseRegion(step, parameterValue);
            }
            else if (parameterKey == "depth" || parameterKey == "thickness")
            {
                step.depth = ParseDouble("depth", parameterValue);
            }
            else if (parameterKey == "material")
            {
                step.material = ParseMaterial("material", parameterValue);
            }
            else
            {
                throw std::runtime_error(
                    "Unknown STEP parameter key: '" + parameterKey + "'"
                );
            }
        }

        return step;
    }

    void AppendDefaultSteps(SimulationConfig& config)
    {
        namespace D = SimulationDefaults;

        auto makeRegionStep =
            [](ProcessStepType type, double depth, MaterialType material)
        {
            ProcessStep step;
            step.type = type;
            step.depth = depth;
            step.material = material;
            step.iStart = D::kRegionIStart;
            step.iEnd = D::kRegionIEnd;
            step.jStart = D::kRegionJStart;
            step.jEnd = D::kRegionJEnd;
            step.hasRegion = true;
            return step;
        };

        config.steps.push_back(makeRegionStep(
            ProcessStepType::Photolithography,
            D::kResistThickness,
            MaterialType::Empty
        ));
        config.steps.push_back(makeRegionStep(
            ProcessStepType::Etch,
            D::kEtchDepth,
            MaterialType::Empty
        ));
        config.steps.push_back(makeRegionStep(
            ProcessStepType::Etch,
            D::kEtchDepth2,
            MaterialType::Empty
        ));
        config.steps.push_back(makeRegionStep(
            ProcessStepType::Deposit,
            D::kDepositDepth,
            MaterialType::Oxide
        ));

        ProcessStep strip;
        strip.type = ProcessStepType::StripResist;
        config.steps.push_back(strip);
    }

    void ResolveAndValidateStep(ProcessStep& step, const SimulationConfig& config)
    {
        if (!step.hasRegion)
        {
            step.iStart = 0;
            step.iEnd = config.nx - 1;
            step.jStart = 0;
            step.jEnd = config.ny - 1;
            step.hasRegion = true;
        }

        if (step.iStart > step.iEnd || step.jStart > step.jEnd)
        {
            throw std::runtime_error(
                "STEP region start index must not exceed end index."
            );
        }

        if (step.iStart < 0 || step.iEnd >= config.nx ||
            step.jStart < 0 || step.jEnd >= config.ny)
        {
            throw std::runtime_error(
                "STEP region indices must be within mesh bounds: "
                "I in [0, " + std::to_string(config.nx - 1) + "], "
                "J in [0, " + std::to_string(config.ny - 1) + "]."
            );
        }

        const std::string label =
            std::string(ProcessStepTypeToString(step.type)) + " step";

        switch (step.type)
        {
        case ProcessStepType::Photolithography:
        case ProcessStepType::Etch:
            if (step.depth <= 0.0)
            {
                throw std::runtime_error(
                    label + " requires a positive depth/thickness."
                );
            }
            break;

        case ProcessStepType::Deposit:
            if (step.depth <= 0.0)
            {
                throw std::runtime_error(
                    label + " requires a positive depth."
                );
            }

            if (step.material == MaterialType::Empty)
            {
                throw std::runtime_error(
                    label + " requires a material (e.g. material=Oxide)."
                );
            }
            break;

        case ProcessStepType::StripResist:
            break;
        }
    }

    void ValidateAndResolveConfig(SimulationConfig& config)
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

        if (config.substrateLayers < 0 ||
            config.substrateLayers > config.nz)
        {
            throw std::runtime_error(
                "SUBSTRATE_LAYERS must be between 0 and NZ ("
                + std::to_string(config.nz) + ")."
            );
        }

        for (ProcessStep& step : config.steps)
        {
            ResolveAndValidateStep(step, config);
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

    config.initialMaterial = SimulationDefaults::kInitialMaterial;

    config.verbose = SimulationDefaults::kVerbose;

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
        else if (key == "INITIAL_MATERIAL")
        {
            config.initialMaterial = ParseMaterial(key, value);
        }
        else if (key == "VERBOSE")
        {
            config.verbose = ParseBool(key, value);
        }
        else if (key == "STEP")
        {
            config.steps.push_back(ParseStep(value));
        }
        else
        {
            throw std::runtime_error("Unknown input key: '" + key + "'");
        }
    }

    if (config.steps.empty())
    {
        AppendDefaultSteps(config);
    }

    ValidateAndResolveConfig(config);

    return config;
}
