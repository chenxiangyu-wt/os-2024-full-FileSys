#ifndef _FILESYS_H
#define _FILESYS_H

// #define exit(a)		return			//建议使用 return         by tangfl

// All Defines
constexpr int BLOCKSIZ = 512; // Size of each block
constexpr int SYSOPENFILE = 40;
constexpr int DIRNUM = 128; // Maximum number of subdirectories in a directory
constexpr int DIRSIZ = 12;	// Directory name length, windows32 int length is 4, changed from 14 to 12
constexpr int PWDSIZ = 12;	// Maximum password length
constexpr int PWDNUM = 32;
constexpr int NOFILE = 20; // Maximum number of files a user can open
constexpr int NADDR = 10;  // i-node storage
constexpr int NHINO = 128; // Hash number, must be power of 2
constexpr int USERNUM = 10;
constexpr int DINODESIZ = 52; // Should be 50 bytes, but adjusted to 52 for memory alignment, changed from 32 to 52

// File system constants
constexpr int DINODEBLK = 32;						  // i-node block count
constexpr int FILEBLK = 512;						  // Data block count
constexpr int NICFREE = 50;							  // Free block stack size in superblock
constexpr int NICINOD = 50;							  // Free i-node array size in superblock
constexpr int DINODESTART = 2 * BLOCKSIZ;			  // i-node start address, leaving 1024 bytes, first for boot, second for superblock
constexpr int DATASTART = (2 + DINODEBLK) * BLOCKSIZ; // Data area start address

// di_mode constants
constexpr int DIEMPTY = 00000; // Empty permission
constexpr int DIFILE = 01000;  // Type: File
constexpr int DIDIR = 02000;   // Type: Directory

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

constexpr int READ = 1;
constexpr int WRITE = 2;
constexpr int EXICUTE = 3;

constexpr int DEFAULTMODE = 00777; // Default permission

// i_flag constants
constexpr int IUPDATE = 00002;

// s_fmod constants
constexpr int SUPDATE = 00001;

// f_flag constants
constexpr int FREAD = 00001;
constexpr int FWRITE = 00002;
constexpr int FAPPEND = 00004;

/* error */
#define DISKFULL 65535

/* fseek origin */
// #define SEEK_SET  		0

struct INode
{
	struct INode *next;					 // 指向下一个 i 节点（用于链表）
	struct INode *prev;					 // 指向前一个 i 节点（用于链表）
	char status_flag;					 // i 节点状态标志
	unsigned int inode_number;			 // i 节点编号
	unsigned int reference_count;		 // 引用计数
	unsigned short link_count;			 // 链接数
	unsigned short mode;				 // 存取权限和文件类型
	unsigned short owner_user_id;		 // 所有者用户 ID
	unsigned short owner_group_id;		 // 所有者组 ID
	unsigned short file_size;			 // 文件大小
	unsigned int block_addresses[NADDR]; // 数据块地址（直接/间接）
};

struct Dinode
{
	unsigned short di_number; /*关联文件数*/
	unsigned short di_mode;	  /*存取权限*/
	unsigned short di_uid;
	unsigned short di_gid;
	unsigned short di_size;		 /*文件大小*/
	unsigned int di_addr[NADDR]; /*物理块号*/
};

// 目录项
struct DirectoryEntry
{
	char name[DIRSIZ];		   // 文件或目录名称
	unsigned int inode_number; // i 节点编号
};

// 目录
struct Directory
{
	struct DirectoryEntry entries[DIRNUM]; // 目录项数组
	unsigned int entry_count;			   // 当前目录中的有效项数
};
struct FileSystem
{
	unsigned short inode_block_count;  /* i 节点块块数 */
	unsigned long data_block_count;	   /* 数据块块数 */
	unsigned int free_block_count;	   /* 空闲块数 */
	unsigned short free_block_pointer; /* 空闲块指针 */
	unsigned int free_blocks[NICFREE]; /* 空闲块堆栈 */

	unsigned int free_inode_count;	   /* 空闲 i 节点数 */
	unsigned short free_inode_pointer; /* 空闲 i 节点指针 */
	unsigned int free_inodes[NICINOD]; /* 空闲 i 节点数组 */
	unsigned int last_allocated_inode; /* 记录的 i 节点 */

	char superblock_modified_flag; /* 超级块修改标志 */
};

struct UserPassword
{
	unsigned short user_id;	 // 用户 ID
	unsigned short group_id; // 组 ID
	char password[PWDSIZ];	 // 用户密码
};

struct InodeHashTableEntry
{
	INode *prev_inode; /*HASH表指针*/
};

struct File
{
	char flag;					  /* 文件操作标志 */
	unsigned int reference_count; /* 引用计数 */
	INode *inode;				  /* 指向内存 i 节点 */
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
extern INode *cur_path_inode;
extern int user_id; /* 用户 ID */
extern char disk[(DINODEBLK + FILEBLK + 2) * BLOCKSIZ];

// all functions
extern INode *iget(unsigned int);
extern void iput(INode *);
extern unsigned int balloc(unsigned int);
extern unsigned int balloc();
extern void bfree(unsigned int);
extern INode *ialloc();
extern void ifree(unsigned int);
extern int namei(const char *);
extern unsigned short iname(const char *);
extern unsigned int access(unsigned int, INode *, unsigned short);
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
