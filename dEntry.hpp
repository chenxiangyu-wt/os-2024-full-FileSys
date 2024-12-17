#ifndef FILESYS_HPP
#define FILESYS_HPP
#include <cstdint>
#include <string>

constexpr uint16_t ENTRY_NUM = 12;
constexpr uint16_t ENTRYNUM = 128;
constexpr uint16_t DENTRY_FILE = 0x4000;
constexpr uint16_t DENTRY_DIR = 0x8000;
constexpr uint16_t DENTRY_EMPTY = 0x0100;

// f_flag constants
constexpr uint16_t FREAD = 0x01;   // 只读
constexpr uint16_t FWRITE = 0x02;  // 可写
constexpr uint16_t FAPPEND = 0x04; // 追加

// mode constants
constexpr int READ = 1;
constexpr int WRITE = 2;
constexpr int EXICUTE = 3;

struct DirectoryEntry
{
    char name[ENTRY_NUM];  // 文件或目录名称
    uint32_t inode_number; // i 节点编号
    uint16_t type;         // 类型：文件或目录（DENTRY_FILE, DENTRY_DIR）;本着增量修改的原则,在这Directory结构体中额外添加type字段，用于标识文件类型
};

struct Directory
{
    DirectoryEntry entries[ENTRYNUM]; // 目录项数组
    int entry_count;                  // 当前目录中的有效项数
};
extern void _dir();
extern void mkdir(const char *);
extern int chdir(const char *dirname);
extern short openFile(int, const char *, char);
extern int creatFile(uint32_t, const char *, uint16_t);
extern uint32_t readFile(int fd, char *buf, uint32_t size);
extern uint32_t writeFile(int fd, char *buf, uint32_t size);
extern void removeFile(const char *);
int namei(const char *filename, uint16_t type);
extern uint16_t iname(const char *);
int find_empty_entry();
std::string get_current_path();

#endif // FILESYS_HPP