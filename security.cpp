#include "file_sys.hpp"
#include "globals.hpp"
#include "security.hpp"
#include "dEntry.hpp"
#include <termios.h>
#include <iostream>
#include <string.h>

using namespace std;
/**********************************************************************************
函数：access
参数：用户ID号；内存节点；要判断的权限
功能：通过用户权限判断用户是否能对该文件进行相应的操作；权限够返回1，权限不够返回0。
***********************************************************************************/
uint32_t access(uint32_t user_id, struct MemoryINode *inode, uint16_t mode)
{
    switch (mode)
    {
    case READ:
        if (inode->mode & ODIREAD) // 文件是所有用户可读行通过；
            return 1;
        if ((inode->mode & GDIREAD) && (users[user_id].group_id == inode->owner_gid)) // 文件本组可读，用户组和文件组一致通过；
            return 1;
        if ((inode->mode & UDIREAD) && (users[user_id].user_id == inode->owner_uid)) // 文件指定用户可读，是该用户通过；
            return 1;
        return 0; // 其他不通过；
    case WRITE:
        if (inode->mode & ODIWRITE)
            return 1;
        if ((inode->mode & GDIWRITE) && (users[user_id].group_id == inode->owner_gid))
            return 1;
        if ((inode->mode & UDIWRITE) && (users[user_id].user_id == inode->owner_uid))
            return 1;
        return 0;
    case EXICUTE:
        if (inode->mode & ODIEXICUTE)
            return 1;
        if ((inode->mode & GDIEXICUTE) && (users[user_id].group_id == inode->owner_gid))
            return 1;
        if ((inode->mode & UDIEXICUTE) && (users[user_id].user_id == inode->owner_uid))
            return 1;
        return 0;
    } // swith
    return 0;
}
void secret_input(char *password, size_t max_length)
{
    char ch;
    size_t i = 0;

    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    while (i < max_length - 1)
    {
        ch = getchar();
        if (ch == 10 || ch == 13)
        {
            break;
        }
        if (ch == 127)
        {
            if (i > 0)
            {
                --i;
                std::cout << "\b \b";
            }
        }
        else
        {
            password[i++] = ch;
            std::cout << "*";
        }
    }

    password[i] = '\0';
    std::cout << std::endl;

    // 恢复终端设置
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}
