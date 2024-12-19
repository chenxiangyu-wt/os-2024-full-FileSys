#include "command_line.hpp"
#include "file_sys.hpp"
#include <sstream>
#include <cstdlib>
#include <cstring>
#include "dEntry.hpp"
#include "security.hpp"

// 构造函数
CommandLine::CommandLine()
{
    command_map["exit"] = [this](const std::vector<std::string> &args)
    { return cmdExit(args); };
    command_map["dir"] = [this](const std::vector<std::string> &args)
    { return cmdDir(args); };
    command_map["ls"] = [this](const std::vector<std::string> &args)
    { return cmdDir(args); };
    command_map["mkdir"] = [this](const std::vector<std::string> &args)
    { return cmdMkdir(args); };
    command_map["cd"] = [this](const std::vector<std::string> &args)
    { return cmdCd(args); };
    command_map["mkfile"] = [this](const std::vector<std::string> &args)
    { return cmdMkfile(args); };
    command_map["del"] = [this](const std::vector<std::string> &args)
    { return cmdDel(args); };
    command_map["write"] = [this](const std::vector<std::string> &args)
    { return cmdWrite(args); };
    command_map["read"] = [this](const std::vector<std::string> &args)
    { return cmdRead(args); };
    command_map["who"] = [this](const std::vector<std::string> &args)
    { return cmdWho(args); };
    command_map["pwd"] = [this](const std::vector<std::string> &args)
    { return cmdPwd(args); };
    command_map["rename"] = [this](const std::vector<std::string> &args)
    { return cmdRename(args); };
    command_map["cp"] = [this](const std::vector<std::string> &args)
    { return cmdCopy(args); };
}

// 输入解析
std::vector<std::string> CommandLine::parseInput(const std::string &input)
{
    std::istringstream iss(input);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token)
    {
        tokens.push_back(token);
    }
    return tokens;
}

int CommandLine::execute(const std::string &input)
{
    std::vector<std::string> args = parseInput(input);
    if (args.empty())
    {
        return SUCC_RETURN;
    }

    auto it = command_map.find(args[0]);
    if (it != command_map.end())
    {
        return it->second(args);
    }
    else
    {
        std::cerr << "未知命令: " << args[0] << std::endl;
        return SUCC_RETURN;
    }
}

// dir 命令
int CommandLine::cmdDir(const std::vector<std::string> &args)
{
    _dir();
    return SUCC_RETURN;
}

// mkdir 命令
int CommandLine::cmdMkdir(const std::vector<std::string> &args)
{
    if (args.size() < 2)
    {
        std::cerr << "mkdir 命令的正确格式为: mkdir <dirname>" << std::endl;
        return SUCC_RETURN;
    }
    mkdir(args[1].c_str());
    return SUCC_RETURN;
}

// cd 命令
int CommandLine::cmdCd(const std::vector<std::string> &args)
{
    if (args.size() < 2)
    {
        std::cerr << "cd 命令的正确格式为: cd <dirname>" << std::endl;
        return SUCC_RETURN;
    }
    auto res = chdir(args[1].c_str());
    if (res)
    {
        std::cout << "目录切换成功！" << std::endl;
    }

    return SUCC_RETURN;
}

// mkfile 命令
int CommandLine::cmdMkfile(const std::vector<std::string> &args)
{
    if (args.size() < 2)
    {
        std::cerr << "mkfile 命令的正确格式为: mkfile <filename> [mode]" << std::endl;
        return SUCC_RETURN;
    }
    std::string filename = args[1];
    uint16_t mode = DEFAULTMODE;
    if (args.size() > 2)
    {
        sscanf(args[2].c_str(), "%ho", &mode);
    }
    mode |= DIFILE | 0700;
    int fd = creatFile(0, filename.c_str(), mode);
    if (fd == -1)
    {
        std::cerr << "创建文件失败！" << std::endl;
        return SUCC_RETURN;
    }
    closeFile(0, fd);
    return SUCC_RETURN;
}

// del 命令
int CommandLine::cmdDel(const std::vector<std::string> &args)
{
    if (args.size() < 2)
    {
        std::cerr << "del 命令的正确格式为: del <filename>" << std::endl;
        return SUCC_RETURN;
    }
    removeFile(args[1].c_str());
    return SUCC_RETURN;
}

// write 命令
int CommandLine::cmdWrite(const std::vector<std::string> &args)
{
    if (args.size() < 4) // 检查参数是否足够
    {
        std::cerr << "用法: write <filename> <data> <size>" << std::endl;
        return SUCC_RETURN;
    }

    std::string filename = args[1];
    std::string data = args[2];
    uint32_t size;

    // 将输入的字节数参数转换为整数
    try
    {
        size = std::stoi(args[3]);
        if (size == 0)
        {
            std::cerr << "错误: 写入字节数必须大于 0。" << std::endl;
            return SUCC_RETURN;
        }
    }
    catch (const std::invalid_argument &)
    {
        std::cerr << "错误: 字节数必须是有效的整数。" << std::endl;
        return SUCC_RETURN;
    }

    // 准备缓冲区：截断或填充数据到指定大小
    char *buf = (char *)malloc(size);
    memset(buf, 0, size); // 默认用空字符填充

    // 复制用户输入的内容，确保不会超过指定字节数
    strncpy(buf, data.c_str(), size);

    // 打开文件
    short mode = WRITE;
    int file_id = openFile(0, filename.c_str(), mode);
    if (file_id == -1)
    {
        std::cerr << "错误: 无法打开文件 '" << filename << "'，请检查路径和权限。" << std::endl;
        free(buf);
        return SUCC_RETURN;
    }

    // 调用 writeFile 写入数据
    uint32_t written_size = writeFile(file_id, buf, size);

    std::cout << written_size << " 字节数据已写入文件 '" << filename << "'。" << std::endl;

    // 释放资源
    free(buf);
    closeFile(0, file_id);
    return SUCC_RETURN;
}

// read 命令
int CommandLine::cmdRead(const std::vector<std::string> &args)
{
    if (args.size() < 3)
    {
        std::cerr << "read 命令的正确格式为: read <filename> <bytes>" << std::endl;
        return SUCC_RETURN;
    }
    std::string filename = args[1];
    uint32_t size = std::stoi(args[2]);

    int fd = openFile(0, filename.c_str(), READ);
    switch (fd)
    {
    case -1:
        std::cerr << "文件不存在！" << std::endl;
        break;
    case -2:
        std::cerr << "权限不足！" << std::endl;
        break;
    default:
        char *buf = (char *)malloc(size + 1);
        size = readFile(fd, buf, size);
        std::cout << size << " bytes 已从文件 " << filename << " 读取到缓冲区." << std::endl;
        free(buf);
        closeFile(0, fd);
        break;
    }
    return SUCC_RETURN;
}

// exit 命令
int CommandLine::cmdExit(const std::vector<std::string> &args)
{
    return EXIT_RETURN;
}

int CommandLine::cmdWho(const std::vector<std::string> &args)
{
    who();
    return SUCC_RETURN;
}

int CommandLine::cmdPwd(const std::vector<std::string> &args)
{
    std::string cur_path = get_current_path();
    std::cout << "当前目录：" << cur_path << std::endl;
    return SUCC_RETURN;
}

int CommandLine::cmdRename(const std::vector<std::string> &args)
{
    if (args.size() < 3)
    {
        std::cerr << "rename 命令的正确格式为: rename <oldname> <newname>" << std::endl;
        return SUCC_RETURN;
    }
    int res = renameFile(args[1].c_str(), args[2].c_str());
    if (res == -1)
    {
        std::cerr << "重命名失败！" << std::endl;
    }
    return SUCC_RETURN;
}

int CommandLine::cmdCopy(const std::vector<std::string> &args)
{
    if (args.size() < 3)
    {
        std::cerr << "cp 命令的正确格式为: cp <src> <dest>" << std::endl;
        return SUCC_RETURN;
    }
    int res = copyFile(args[1].c_str(), args[2].c_str());
    if (res == -1)
    {
        std::cerr << "复制文件失败！" << std::endl;
    }
    return SUCC_RETURN;
}