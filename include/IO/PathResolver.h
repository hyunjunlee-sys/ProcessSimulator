#ifndef PATHRESOLVER_H
#define PATHRESOLVER_H

#include <string>

class PathResolver
{
public:
    static std::string ResolveReadablePath(const std::string& path);
    static std::string ResolveWritablePath(const std::string& path);
};

#endif
