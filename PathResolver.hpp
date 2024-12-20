#ifndef PATH_RESOLVER_HPP
#define PATH_RESOLVER_HPP

#include <string>
#include <vector>

class PathResolver
{
public:
    static std::string resolve(const std::string &basePath, const std::string &targetPath);

    static std::vector<std::string> splitAndNormalize(const std::string &path);

private:
    static std::string buildPath(const std::vector<std::string> &stack);
};

#endif // PATH_RESOLVER_HPP
