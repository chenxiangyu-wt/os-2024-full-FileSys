#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <cstdint>
#include "file_sys.hpp"
#include "security.hpp"
#include "iNode.hpp"
#include "dEntry.hpp"

// 哈希 i-node 表
extern InodeHashTableEntry hinode[NHINO];

// 当前目录
extern Directory dir;

// 系统打开文件表
extern File system_opened_file[SYSTEM_MAX_OPEN_FILE_NUM];

// 超级块
extern FileSystem fileSystem;

// 用户密码信息
extern UserPassword pwd[PWDNUM];

// 用户上下文
extern UserContext users[USERNUM];

// 当前路径对应的 i-node
extern MemoryINode *cwd;

// 当前用户 ID
extern int user_id;

// 模拟磁盘的内存区域
extern uint8_t disk[DISK_SIZE];

// 声明 DISK_FILE（不要赋值）
extern const char *DISK_FILE;

#endif // GLOBALS_HPP
