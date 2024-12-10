#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include "file_sys.hpp"
#include "security.hpp"
#include "file_sys.hpp"
#include "iNode.hpp"
#include "dEntry.hpp"

// 哈希 i-node 表
extern InodeHashTableEntry hinode[NHINO];

// 当前目录
extern Directory dir; /* 当前目录 (在内存中全部读入) */

// 系统打开文件表
extern File sys_ofile[SYSOPENFILE];

// 超级块（文件系统元数据）
extern FileSystem fileSystem;

// 用户密码信息
extern UserPassword pwd[PWDNUM];

// 用户上下文
extern UserContext user[USERNUM];

// 当前路径对应的 i-node
extern MemoryINode *cur_path_inode;

// 当前用户 ID
extern int user_id;

// 模拟磁盘的内存区域
extern char disk[(DISK_INODE_AREA_SIZE + DATA_BLOCK_AREA_SIZE + 2) * BLOCK_SIZE];

#endif // GLOBALS_HPP