#ifndef _FILESYS_H
#define _FILESYS_H

// #define exit(a)		return			//建议使用 return         by tangfl

// All Defines
#define BLOCKSIZ 512 // 每个块的大小
#define SYSOPENFILE 40
#define DIRNUM 128 // 一个目录下 最多 子目录数
#define DIRSIZ 12  // 目录名长度 windows32下int长度为4 xiao 14->12
#define PWDSIZ 12  // 密码最大长度
#define PWDNUM 32
#define NOFILE 20 // 用户最多打开文件数
#define NADDR 10  // i节点内存储
#define NHINO 128 // hash数  ?????????/* must be power of 2 */
#define USERNUM 10
#define DINODESIZ 52 //?????? int=4 故多了2*NADRR 但源代码中为long，该做short 应该为50字节，此处可能系统为了对齐内存，故如此调整xiao 32->52

/*filesys*/
#define DINODEBLK 32						   // i节点占用的块数namei
#define FILEBLK 512							   // 数据块数
#define NICFREE 50							   // 超级块内空闲块堆栈大小
#define NICINOD 50							   // 超级块内空闲i节点数组大小
#define DINODESTART (2 * BLOCKSIZ)			   // i节点开始地址 空出1024，第一个为引导区，第二块为超级块
#define DATASTART ((2 + DINODEBLK) * BLOCKSIZ) // 数据区开始地址 DINODESTART+DINODEBLK*BLOCKSIZ	/*d:17408    0x4400*/

/*di_mode*/
#define DIEMPTY 00000 // 空权限
#define DIFILE 01000  // 类型 文件
#define DIDIR 02000	  // 类型 目录

#define UDIREAD 00001 // 用户权限
#define UDIWRITE 00002
#define UDIEXICUTE 00004
#define GDIREAD 00010 // 用户组权限
#define GDIWRITE 00020
#define GDIEXICUTE 00040
#define ODIREAD 00100 // pubilc权限
#define ODIWRITE 00200
#define ODIEXICUTE 00400

#define READ 1
#define WRITE 2
#define EXICUTE 3

#define DEFAULTMODE 00777 // 默认权限

/* i_flag */
#define IUPDATE 00002

/* s_fmod */
#define SUPDATE 00001

/* f_flag */
#define FREAD 00001
#define FWRITE 00002
#define FAPPEND 00004

// Includes
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

/* error */
#define DISKFULL 65535

/* fseek origin */
// #define SEEK_SET  		0

struct INode
{
	struct INode *i_forw;
	struct INode *i_back;
	char i_flag;
	unsigned int i_ino;		  /*磁盘i 节点标志*/
	unsigned int i_count;	  /*引用计数*/
	unsigned short di_number; /*关联文件数。当为0 时，则删除该文件*/
	unsigned short di_mode;	  /*存取权限*/
	unsigned short di_uid;
	unsigned short di_gid;
	unsigned short di_size;		 /*文件大小*/
	unsigned int di_addr[NADDR]; /*物理块号*/
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

struct Direct
{
	char d_name[DIRSIZ];
	unsigned int d_ino;
};

struct FileSystem
{
	unsigned short s_isize;		  /* i 节点块块数 */
	unsigned long s_fsize;		  /* 数据块块数 */
	unsigned int s_nfree;		  /* 空闲块数 */
	unsigned short s_pfree;		  /* 空闲块指针 */
	unsigned int s_free[NICFREE]; /* 空闲块堆栈 */

	unsigned int s_ninode;		   /* 空闲 i 节点数 */
	unsigned short s_pinode;	   /* 空闲 i 节点指针 */
	unsigned int s_inode[NICINOD]; /* 空闲 i 节点数组 */
	unsigned int s_rinode;		   /* 记录的 i 节点 */

	char s_fmod; /* 超级块修改标志 */
};

struct Pwd
{
	unsigned short p_uid;
	unsigned short p_gid;
	char password[PWDSIZ];
};

struct Dir
{
	struct Direct direct[DIRNUM];
	unsigned int size; /* 当前目录大小 */
};

struct Hinode
{
	struct INode *i_forw; /*HASG表指针*/
};

struct File
{
	char f_flag;		   /* 文件操作标志 */
	unsigned int f_count;  /* 引用计数 */
	struct INode *f_inode; /* 指向内存 i 节点 */
	unsigned long f_off;   /* 读/写字符指针偏移量 */
};

struct User
{
	unsigned short u_default_mode;
	unsigned short u_uid;
	unsigned short u_gid;
	unsigned short u_ofile[NOFILE]; /*用户打开文件表*/
};

// all variables
extern struct Hinode hinode[NHINO];
extern struct Dir Dir; /*当前目录(在内存中全部读入)*/
extern struct File sys_ofile[SYSOPENFILE];
extern struct FileSystem FileSystem; /*内存中的超级块*/
extern struct Pwd pwd[PWDNUM];
extern struct User user[USERNUM];
// extern struct file     *fd;           /*the file system column of all the system*/    //xiao
extern struct INode *cur_path_inode;
extern int user_id; /* 用户 ID */
extern char disk[(DINODEBLK + FILEBLK + 2) * BLOCKSIZ];

// all functions
extern struct INode *iget(unsigned int);
extern void iput(struct INode *);
extern unsigned int balloc(unsigned int);
extern unsigned int balloc();
extern void bfree(unsigned int);
extern struct INode *ialloc();
extern void ifree(unsigned int);
extern int namei(const char *);
extern unsigned short iname(const char *);
extern unsigned int access(unsigned int, struct INode *, unsigned short);
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
