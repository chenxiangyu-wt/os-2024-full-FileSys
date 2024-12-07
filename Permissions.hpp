#ifndef PERMISSIONS_HPP
#define PERMISSIONS_HPP

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

struct UserPassword
{
    unsigned short user_id;  // 用户 ID
    unsigned short group_id; // 组 ID
    char password[PWDSIZ];   // 用户密码
};

#endif // PERMISSIONS_HPP