#ifndef _FILESYS_H
#define _FILESYS_H
#include <cstdint>
#include "iNode.hpp"
#include "dEntry.hpp"
#include "security.hpp"

// All Defines
constexpr int BLOCK_SIZE = 512; // Size of each block
constexpr int SYSTEM_MAX_OPEN_FILE_NUM = 40;

constexpr int NHINO = 128; // Hash number, must be power of 2

// File system constants
constexpr int DISK_INODE_AREA_SIZE = 16;									// i-node block count
constexpr int DATA_BLOCK_AREA_SIZE = 64;									// Data block count
constexpr int NICFREE = 50;													// Free block stack size in superblock
constexpr int NICINOD = 50;													// Free i-node array size in superblock
constexpr int DISK_INODE_START_POINTOR = 2 * BLOCK_SIZE;					// i-node start address, leaving 1024 bytes, first for boot, second for superblock
constexpr int DATA_START_POINTOR = (2 + DISK_INODE_AREA_SIZE) * BLOCK_SIZE; // Data area start address
constexpr int DISK_SIZE = (DISK_INODE_AREA_SIZE + DATA_BLOCK_AREA_SIZE + 2) * BLOCK_SIZE;
constexpr int DISK_INODE_SIZE = sizeof(DiskINode); // Should be 50 bytes, but adjusted to 52 for memory alignment, changed from 32 to 52
constexpr int MEMORY_INODE_SIZE = sizeof(MemoryINode);

// s_fmod constants
constexpr int SUPDATE = 00001;

/* error */
constexpr int DISKFULL = 65535; /* 磁盘已满 */

/* fseek origin */
// #define SEEK_SET  		0
struct FileSystem
{
	uint16_t inode_block_count;	   /* i 节点块块数 */
	uint64_t data_block_count;	   /* 数据块块数 */
	uint32_t free_block_count;	   /* 空闲块数 */
	uint16_t free_block_pointer;   /* 空闲块指针 */
	uint32_t free_blocks[NICFREE]; /* 空闲块堆栈 */

	uint32_t free_inode_count;	   /* 空闲 i 节点数 */
	uint16_t free_inode_pointer;   /* 空闲 i 节点指针 */
	uint32_t free_inodes[NICINOD]; /* 空闲 i 节点数组 */
	uint32_t last_allocated_inode; /* 记录的 i 节点 */

	uint8_t superblock_modified_flag; /* 超级块修改标志 */
};

struct File
{
	char flag;				  /* 文件操作标志 */
	uint32_t reference_count; /* 引用计数 */
	MemoryINode *inode;		  /* 指向内存 i 节点 */
	unsigned long offset;	  /* 读/写字符指针偏移量 */
};

// all functions
extern uint32_t balloc(uint32_t);
extern uint32_t balloc();
extern void bfree(uint32_t);

extern void install();
extern void format();
extern void close(uint32_t, uint16_t);
extern void halt();

#endif
