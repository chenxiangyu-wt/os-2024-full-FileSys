#ifndef FILESYS_HPP
#define FILESYS_HPP
#include <cstdint>

constexpr uint16_t ENTRYNAMELEN = 12;
constexpr uint16_t ENTRYNUM = 128;
constexpr uint8_t DENTRY_FILE = 0;
constexpr uint8_t DENTRY_DIR = 1;
constexpr uint8_t DENTRY_EMPTY = 2;

struct DirectoryEntry
{
    char name[ENTRYNAMELEN];   // 文件或目录名称
    unsigned int inode_number; // i 节点编号
    uint8_t type;              // 文件类型
};

struct Directory
{
    DirectoryEntry entries[ENTRYNUM]; // 目录项数组
    unsigned int entry_count;         // 当前目录中的有效项数
};

#endif // FILESYS_HPP