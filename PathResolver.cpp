#include "PathResolver.hpp"
#include <sstream>
#include <algorithm>

// 去除路径末尾的 '/'
std::string removeTrailingSlash(const std::string &path)
{
    if (path.size() > 1 && path.back() == '/')
    {
        return path.substr(0, path.size() - 1);
    }
    return path;
}

std::string PathResolver::resolve(const std::string &basePath, const std::string &targetPath)
{
    std::vector<std::string> pathStack;

    std::string cleanBasePath = removeTrailingSlash(basePath);
    std::string cleanTargetPath = removeTrailingSlash(targetPath);

    // 处理绝对路径和相对路径
    if (!cleanTargetPath.empty() && cleanTargetPath[0] == '/')
    {
        // 绝对路径：从根目录开始
        pathStack = splitAndNormalize(cleanTargetPath);
    }
    else
    {
        // 相对路径：从 cleanBasePath 开始拼接 cleanTargetPath
        pathStack = splitAndNormalize(cleanBasePath);
        std::vector<std::string> targetStack = splitAndNormalize(cleanTargetPath);
        pathStack.insert(pathStack.end(), targetStack.begin(), targetStack.end());
    }

    return buildPath(pathStack);
}

std::string PathResolver::buildPath(const std::vector<std::string> &stack)
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
std::vector<std::string> PathResolver::splitAndNormalize(const std::string &path)
{
    std::vector<std::string> stack;
    std::istringstream iss(path);
    std::string token;

    while (std::getline(iss, token, '/'))
    {
        if (token.empty() || token == ".")
        {
            continue;
        }
        if (token == "..")
        {
            if (!stack.empty())
            {
                stack.pop_back();
            }
        }
        else
        {
            stack.push_back(token);
        }
    }

    return stack;
}