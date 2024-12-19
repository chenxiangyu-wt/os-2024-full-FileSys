#ifndef COMMAND_LINE_HPP
#define COMMAND_LINE_HPP

#include <unordered_map>
#include <string>
#include <vector>
#include <functional>
#include <iostream>

constexpr int SUCC_RETURN = 1;
constexpr int EXIT_RETURN = 0;

class CommandLine
{
private:
    // 命令映射
    std::unordered_map<std::string, std::function<int(const std::vector<std::string> &)>> command_map;

    // 输入解析
    std::vector<std::string> parseInput(const std::string &input);

    // 各种命令的实现
    int cmdDir(const std::vector<std::string> &args);
    int cmdMkdir(const std::vector<std::string> &args);
    int cmdCd(const std::vector<std::string> &args);
    int cmdMkfile(const std::vector<std::string> &args);
    int cmdDel(const std::vector<std::string> &args);
    int cmdWrite(const std::vector<std::string> &args);
    int cmdRead(const std::vector<std::string> &args);
    int cmdExit(const std::vector<std::string> &args);
    int cmdPwd(const std::vector<std::string> &args);
    int cmdWho(const std::vector<std::string> &args);
    int cmdRename(const std::vector<std::string> &args);

public:
    CommandLine();
    int execute(const std::string &input);
};

#endif // COMMAND_LINE_HPP