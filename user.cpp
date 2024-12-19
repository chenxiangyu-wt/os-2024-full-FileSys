#include "globals.hpp"
#include "security.hpp"
#include <iostream>

UserInfo get_user_info()
{
    UserInfo info;
    info.user_id = user_id;
    info.group_id = user[user_id].group_id;
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