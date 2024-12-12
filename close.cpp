#include <stdio.h>
#include "file_sys.hpp"
#include "globals.hpp"

/******************************************************
函数：close
功能：关闭打开文件时，修改用户打开表和系统打开表中的纪录信息
*******************************************************/
void close(uint32_t user_id, uint16_t cfd)
{
	struct MemoryINode *inode;
	inode = system_opened_file[user[user_id].open_files[cfd]].inode;
	iput(inode);
	system_opened_file[user[user_id].open_files[cfd]].reference_count--;
	user[user_id].open_files[cfd] = SYSOPENFILE + 1;
	return;
}
