#include <stdio.h>
#include "filesys.hpp"

/******************************************************
函数：close
功能：关闭打开文件时，修改用户打开表和系统打开表中的纪录信息
*******************************************************/
void close(unsigned int user_id, unsigned short cfd)
{
	struct MemoryINode *inode;
	inode = sys_ofile[user[user_id].open_files[cfd]].inode;
	iput(inode);
	sys_ofile[user[user_id].open_files[cfd]].reference_count--;
	user[user_id].open_files[cfd] = SYSOPENFILE + 1;
	return;
}
