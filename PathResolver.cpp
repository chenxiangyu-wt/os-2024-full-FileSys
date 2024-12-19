#include "PathResolver.hpp"
#include <sstream>

// 解析路径，返回标准化后的路径
std::string PathResolver::resolvePath(const std::string &basePath, const std::string &targetPath)
{
    if (targetPath.empty())
        return basePath;

    std::vector<std::string> pathStack;

    // 处理绝对路径和相对路径
    if (targetPath[0] == '/')
    {
        // 如果 targetPath 是绝对路径，直接从根目录开始
        splitAndResolve(targetPath, pathStack);
    }
    else
    {
        // 否则从 basePath 开始解析
        splitAndResolve(basePath, pathStack);
        splitAndResolve(targetPath, pathStack);
    }

    // 构建最终路径
    return buildFinalPath(pathStack);
}

// 分割路径并解析
void PathResolver::splitAndResolve(const std::string &path, std::vector<std::string> &stack)
{
    std::istringstream ss(path);
    std::string part;

    while (std::getline(ss, part, '/'))
    {
        if (part.empty() || part == ".")
        {
            // 忽略空字符串和当前目录
            continue;
        }
        else if (part == "..")
        {
            // 返回上一级目录（弹栈）
            if (!stack.empty())
                stack.pop_back();
        }
        else
        {
            // 普通目录，加入栈
            stack.push_back(part);
        }
    }
}

// 构建最终路径
std::string PathResolver::buildFinalPath(const std::vector<std::string> &stack)
{
    if (stack.empty())
        return "/"; // 根目录

    std::string result;
    for (const auto &dir : stack)
    {
        result += "/" + dir;
    }
    return result;
}
