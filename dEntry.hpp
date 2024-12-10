#ifndef FILESYS_HPP
#define FILESYS_HPP
#include <cstdint>

constexpr uint16_t ENTRY_NAME_LEN = 12;
constexpr uint16_t ENTRYNUM = 128;
constexpr uint8_t DENTRY_FILE = 0;
constexpr uint8_t DENTRY_DIR = 1;
constexpr uint8_t DENTRY_EMPTY = 2;

// f_flag constants
constexpr int FREAD = 00001;
constexpr int FWRITE = 00002;
constexpr int FAPPEND = 00004;

// mode constants
constexpr int READ = 1;
constexpr int WRITE = 2;
constexpr int EXICUTE = 3;

struct DirectoryEntry
{
    char name[ENTRY_NAME_LEN]; // 文件或目录名称
    uint32_t inode_number;     // i 节点编号
    uint8_t type;              // 文件类型
};

struct Directory
{
    DirectoryEntry entries[ENTRYNUM]; // 目录项数组
    uint32_t entry_count;             // 当前目录中的有效项数
};
extern void _dir();
extern void mkdir(const char *);
extern void chdir(const char *);
extern short open(int, const char *, char);
extern int creat(uint32_t, const char *, uint_16);
extern uint32_t read(int fd, char *buf, uint32_t size);
extern uint32_t write(int fd, char *buf, uint32_t size);
extern void removeFile(const char *);
#endif // FILESYS_HPP