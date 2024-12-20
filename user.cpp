#include <iostream>
#include <cstring>
#include "globals.hpp"
#include "security.hpp"

UserInfo get_user_info()
{
    UserInfo info;
    info.user_id = user_id;
    info.group_id = users[user_id].group_id;
    info.password = pwd[user_id].password;
    return info;
}

int who()
{
    auto info = get_user_info();
    std::cout << "User ID: " << info.user_id << std::endl;
    std::cout << "Group ID: " << info.group_id << std::endl;
    std::cout << "Password: " << info.password << std::endl;
    return 0;
}

int addUser(const char *username, const char *password)
{
    // 查找空闲用户槽
    int free_slot = -1;
    for (int i = 0; i < PWDNUM; i++)
    {
        if (pwd[i].user_id == 0)
        {
            free_slot = i;
            break;
        }
    }
    if (free_slot == -1)
    {
        std::cerr << "用户表已满！" << std::endl;
        return 1;
    }
    // 检查用户名是否已存在
    for (int i = 0; i < PWDNUM; i++)
    {
        if (strcmp(pwd[i].password, username) == 0)
        {
            std::cerr << "用户名已存在！" << std::endl;
            return 1;
        }
    }
    // 添加用户
    pwd[free_slot].user_id = free_slot;
    pwd[free_slot].group_id = 0;
    strcpy(pwd[free_slot].password, password);
    // 写入磁盘

    return 1;
}