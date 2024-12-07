#ifndef _FILESYS_H
#define _FILESYS_H
#include <cstdint>

#include "iNode.hpp"
#include "permissions.hpp"
#include "directory.hpp"

// All Defines
constexpr int BLOCK_SIZE = 512; // Size of each block
constexpr int SYSOPENFILE = 40;

constexpr int NHINO = 128;						   // Hash number, must be power of 2
constexpr int DISK_INODE_SIZE = sizeof(DiskINode); // Should be 50 bytes, but adjusted to 52 for memory alignment, changed from 32 to 52

// File system constants
constexpr int DISK_INODE_AREA_SIZE = 128;									// i-node block count
constexpr int DATA_BLOCK_AREA_SIZE = 512;									// Data block count
constexpr int NICFREE = 50;													// Free block stack size in superblock
constexpr int NICINOD = 50;													// Free i-node array size in superblock
constexpr int DISK_INODE_START_POINTOR = 2 * BLOCK_SIZE;					// i-node start address, leaving 1024 bytes, first for boot, second for superblock
constexpr int DATA_START_POINTOR = (2 + DISK_INODE_AREA_SIZE) * BLOCK_SIZE; // Data area start address
constexpr int DISK_SIZE = (DISK_INODE_AREA_SIZE + DATA_BLOCK_AREA_SIZE + 2) * BLOCK_SIZE;

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

struct InodeHashTableEntry
{
	MemoryINode *prev_inode; /*HASH表指针*/
};

struct File
{
	char flag;					  /* 文件操作标志 */
	unsigned int reference_count; /* 引用计数 */
	MemoryINode *inode;			  /* 指向内存 i 节点 */
	unsigned long offset;		  /* 读/写字符指针偏移量 */
};

struct UserContext
{
	unsigned short default_mode;	   /* 默认文件权限模式 */
	unsigned short user_id;			   /* 用户 ID */
	unsigned short group_id;		   /* 用户组 ID */
	unsigned short open_files[NOFILE]; /* 用户打开文件表 */
};

// all variables
extern InodeHashTableEntry hinode[NHINO];
extern Directory dir; /*当前目录(在内存中全部读入)*/
extern File sys_ofile[SYSOPENFILE];
extern FileSystem fileSystem; /*内存中的超级块*/
extern UserPassword pwd[PWDNUM];
extern UserContext user[USERNUM];
// extern struct file     *fd;           /*the file system column of all the system*/    //xiao
extern MemoryINode *cur_path_inode;
extern int user_id; /* 用户 ID */
extern char disk[(DISK_INODE_AREA_SIZE + DATA_BLOCK_AREA_SIZE + 2) * BLOCK_SIZE];

// all functions
extern MemoryINode *iget(unsigned int);
extern void iput(MemoryINode *);
extern unsigned int balloc(unsigned int);
extern unsigned int balloc();
extern void bfree(unsigned int);
extern MemoryINode *ialloc();
extern void ifree(unsigned int);
extern int namei(const char *);
extern unsigned short iname(const char *);
extern unsigned int access(unsigned int, MemoryINode *, unsigned short);
extern void _dir();
extern void mkdir(const char *);
extern void chdir(const char *);
extern short open(int, const char *, char);
extern int creat(unsigned int, const char *, unsigned short);
extern unsigned int read(int fd, char *buf, unsigned int size);
extern unsigned int write(int fd, char *buf, unsigned int size);
int login(unsigned short uid, const char *passwd);
extern int logout(unsigned short);
extern void install();
extern void format();
extern void close(unsigned int, unsigned short);
extern void halt();
extern void removeFile(const char *);
extern int shell(int user_id, char *str);

#endif
