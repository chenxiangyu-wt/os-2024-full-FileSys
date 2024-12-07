#include <stdio.h>
#include "file_sys.hpp"
#include "globals.hpp"
#include "dEntry.hpp"
/*********************************************************************
函数：creat
功能：创建文件，存在且可写则覆盖，否则申请i节点，并打开该文件，返回文件指针
**********************************************************************/
int creat(unsigned int user_id, const char *filename, unsigned short mode)
{
	struct MemoryINode *inode;
	int dirid, di_ith;
	uint32_t i, j;

	dirid = namei(filename);
	if (dirid != -1)
	{ // 如果存在同名文件/目录
		inode = iget(dir.entries[dirid].inode_number);
		if (!(inode->mode & DIFILE))
		{ // 如果不是文件
			printf("存在同名目录！\n");
		}
		if (access(user_id, inode, WRITE) == 0)
		{
			iput(inode);
			printf("\n creat access not allowed \n");
			return -1;
		}
		j = inode->file_size % 512 ? 1 : 0;
		for (i = 0; i < inode->file_size / BLOCK_SIZE + j; i++)
			bfree(inode->block_addresses[i]);

		for (i = 0; i < SYSOPENFILE; i++)
		{
			if (sys_ofile[i].reference_count != 0 && sys_ofile[i].inode == inode)
			{
				sys_ofile[i].offset = 0;
			}
		}
		iput(inode);
		return open(user_id, filename, WRITE);
	}
	else
	{
		inode = ialloc();
		di_ith = iname(filename);

		dir.entry_count++;
		dir.entries[di_ith].inode_number = inode->status_flag;
		inode->mode = mode;
		inode->owner_uid = user[user_id].user_id;
		inode->owner_gid = user[user_id].group_id;
		inode->file_size = 0;
		inode->reference_count = 1; // liwen change to 1
		iput(inode);
		return open(user_id, filename, WRITE);
	}
	return 0;
}
