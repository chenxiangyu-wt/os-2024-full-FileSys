#ifndef PERMISSIONS_HPP
#define PERMISSIONS_HPP
#define STDIN_FILENO 0
#include "iNode.hpp"
#include <stdio.h>

constexpr int NOFILE = 20; // Maximum number of files a user can open
constexpr int USERNUM = 10;
constexpr int PWDSIZ = 12; // Maximum password length
constexpr int PWDNUM = 32;
// User permissions
constexpr int UDIREAD = 00001;
constexpr int UDIWRITE = 00002;
constexpr int UDIEXICUTE = 00004;

// Group permissions
constexpr int GDIREAD = 00010;
constexpr int GDIWRITE = 00020;
constexpr int GDIEXICUTE = 00040;

// Public permissions
constexpr int ODIREAD = 00100;
constexpr int ODIWRITE = 00200;
constexpr int ODIEXICUTE = 00400;

constexpr int DEFAULTMODE = 00777; // Default permission

struct UserPassword
{
    uint16_t user_id;      // 用户 ID
    uint16_t group_id;     // 组 ID
    char password[PWDSIZ]; // 用户密码
};

struct UserContext
{
    uint16_t default_mode;       /* 默认文件权限模式 */
    uint16_t user_id;            /* 用户 ID */
    uint16_t group_id;           /* 用户组 ID */
    uint16_t open_files[NOFILE]; /* 用户打开文件表 */
    // MemoryINode *cwd;            /* 当前工作目录的内存 i-node 指针 */
};

struct UserInfo
{
    uint16_t user_id;  // 用户 ID
    uint16_t group_id; // 组 ID
    std::string password;
};

extern uint32_t access(uint32_t, MemoryINode *, uint16_t);
extern int login(uint16_t uid, const char *passwd);
extern int logout(uint16_t);
void secret_input(char *password, size_t max_length);
extern UserInfo get_user_info();
extern int who();

#endif // PERMISSIONS_HPP