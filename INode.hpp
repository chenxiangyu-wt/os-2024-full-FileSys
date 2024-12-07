#ifndef INODE_HPP
#define INODE_HPP
#include <cstdint>

// i_flag constants
constexpr int IUPDATE = 00002;

constexpr int ADDRESS_POINTOR_NUM = 13; // Number of block addresses in an i-node,12个磁盘快地址与一个间接块的地址
constexpr int DIEMPTY = 00000;          // Empty permission
constexpr int DIFILE = 01000;           // Type: File
constexpr int DIDIR = 02000;            // Type: Directory

struct DiskINode
{
    uint16_t mode;                                 // 文件类型和权限
    uint16_t owner_uid;                            // 所有者用户 ID
    uint16_t owner_gid;                            // 所有者组 ID
    uint32_t file_size;                            // 文件大小（字节）
    uint32_t block_addresses[ADDRESS_POINTOR_NUM]; // 数据块地址数组（直接/间接地址）
    // uint16_t reference_count;             // 硬链接计数
    // uint32_t access_time;            // 最后访问时间（时间戳）
    // uint32_t modification_time;      // 最后修改时间（时间戳）
    // uint32_t creation_time;          // 文件创建时间（时间戳）
};

struct MemoryINode
{
    MemoryINode *next; // 指向下一个 i 节点（用于链表）
    MemoryINode *prev; // 指向前一个 i 节点（用于链表）

    uint8_t status_flag;        // 状态标志（状态标志通常不会太复杂，8 位足够）
    uint32_t disk_inode_number; // 磁盘 i 节点编号
    uint32_t reference_count;   // 引用计数

    uint16_t mode;      // 存取权限和文件类型
    uint16_t owner_uid; // 所有者用户 ID
    uint16_t owner_gid; // 所有者组 ID
    uint32_t file_size; // 文件大小（以字节为单位，扩展到 32 位以支持大文件）

    uint32_t block_addresses[ADDRESS_POINTOR_NUM]; // 数据块地址数组（直接/间接地址）
};

#endif // INODE_HPP