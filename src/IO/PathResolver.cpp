#include "IO/PathResolver.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>

namespace
{
    bool FileExists(const std::string& path)
    {
        std::ifstream file(path);
        return file.good();
    }

    std::vector<std::string> BuildCandidatePaths(const std::string& path)
    {
        return {
            path,
            "../" + path
        };
    }
}

std::string PathResolver::ResolveReadablePath(const std::string& path)
{
    for (const std::string& candidate : BuildCandidatePaths(path))
    {
        if (FileExists(candidate))
        {
            return candidate;
        }
    }

    throw std::runtime_error(
        "Failed to locate input file: " + path +
        " (also tried ../" + path + ")"
    );
}

std::string PathResolver::ResolveWritablePath(const std::string& path)
{
    namespace fs = std::filesystem;

    // Prefer a candidate whose parent directory already exists, so the
    // output lands next to the existing project structure regardless of
    // whether the program runs from the project root or a subdirectory.
    for (const std::string& candidate : BuildCandidatePaths(path))
    {
        const fs::path filePath(candidate);

        if (!filePath.has_parent_path() ||
            fs::exists(filePath.parent_path()))
        {
            return candidate;
        }
    }

    const fs::path filePath(path);

    std::error_code errorCode;
    fs::create_directories(filePath.parent_path(), errorCode);

    if (errorCode)
    {
        throw std::runtime_error(
            "Failed to create output directory for: " + path
        );
    }

    return path;
}
