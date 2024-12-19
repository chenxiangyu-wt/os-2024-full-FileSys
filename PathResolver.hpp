#ifndef PATH_RESOLVER_H
#define PATH_RESOLVER_H

#include <string>
#include <vector>

class PathResolver
{
public:
    // 解析路径，返回标准化后的路径
    static std::string resolvePath(const std::string &basePath, const std::string &targetPath);

private:
    // 分割路径并解析
    static void splitAndResolve(const std::string &path, std::vector<std::string> &stack);

    // 构建最终路径
    static std::string buildFinalPath(const std::vector<std::string> &stack);
};

#endif // PATH_RESOLVER_H
