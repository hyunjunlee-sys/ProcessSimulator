#include "Process/ProcessSimulator.h"
#include "IO/InputReader.h"
#include "IO/PathResolver.h"

#include <iostream>
#include <exception>
#include <string>

namespace
{
    void PrintUsage(const char* programName)
    {
        std::cout << "Usage: "
                  << programName
                  << " [input_file] [output_csv]\n"
                  << "Defaults: input/process.txt output/simulation_result.csv\n";
    }
}

int main(int argc, char* argv[])
{
    try
    {
        std::string inputPath = "input/process.txt";
        std::string outputPath = "output/simulation_result.csv";

        if (argc > 3)
        {
            PrintUsage(argv[0]);
            return 1;
        }

        if (argc >= 2)
        {
            inputPath = argv[1];
        }

        if (argc == 3)
        {
            outputPath = argv[2];
        }

        inputPath = PathResolver::ResolveReadablePath(inputPath);
        outputPath = PathResolver::ResolveWritablePath(outputPath);

        InputReader inputReader;
        const SimulationConfig config = inputReader.Read(inputPath);

        ProcessSimulator simulator(
            config.nx,
            config.ny,
            config.nz,
            config.dx,
            config.dy,
            config.dz,
            config.initialMaterial,
            config.substrateLayers
        );

        simulator.RunProcess(config);
        simulator.ExportResult(outputPath);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Simulation error: "
                  << e.what()
                  << "\n";

        return 1;
    }

    return 0;
}
