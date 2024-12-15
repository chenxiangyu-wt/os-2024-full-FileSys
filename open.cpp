#include <stdio.h>
#include "file_sys.hpp"
#include "globals.hpp"

short open(int user_id, const char *filename, char openmode)
{
	int disk_inode_id;
	MemoryINode *inode;
	uint32_t sys_file_index, user_fd_index;

	// 1. 查找目标文件的 inode
	disk_inode_id = namei(filename, DENTRY_FILE);
	if (disk_inode_id == 0)
	{
		printf("\nFile does not exist!!!\n");
		return -1;
	}

	inode = iget(dir.entries[disk_inode_id].inode_number);
	if (!(inode->mode & DIFILE))
	{
		printf("%s is not a file!!!\n", filename);
		iput(inode);
		return -1;
	}

	// 2. 检查用户权限
	if (!access(user_id, inode, openmode))
	{
		printf("\nAccess denied: No permission to open file.\n");
		iput(inode);
		return -1;
	}

	// 3. 查找系统打开文件表的空闲位置
	for (sys_file_index = 0; sys_file_index < SYSTEM_MAX_OPEN_FILE_NUM; sys_file_index++)
	{
		if (system_opened_file[sys_file_index].reference_count == 0)
		{
			break;
		}
	}
	if (sys_file_index == SYSTEM_MAX_OPEN_FILE_NUM)
	{
		printf("\nSystem open file table is full.\n");
		iput(inode);
		return -1;
	}

	// 4. 查找用户打开文件表的空闲位置
	for (user_fd_index = 0; user_fd_index < NOFILE; user_fd_index++)
	{
		if (user[user_id].open_files[user_fd_index] == SYSTEM_MAX_OPEN_FILE_NUM + 1)
		{
			break;
		}
	}
	if (user_fd_index == NOFILE)
	{
		printf("\nUser open file table is full.\n");
		iput(inode);
		return -1;
	}

	// 5. 更新系统和用户文件表
	system_opened_file[sys_file_index].inode = inode;
	system_opened_file[sys_file_index].flag = openmode;
	system_opened_file[sys_file_index].reference_count = 1;

	if (openmode & FAPPEND)
	{
		system_opened_file[sys_file_index].offset = inode->file_size;
	}
	else
	{
		system_opened_file[sys_file_index].offset = 0;
	}

	user[user_id].open_files[user_fd_index] = sys_file_index; // 关联系统文件表索引

	// printf("File '%s' opened successfully. User FD: %d\n", filename, user_fd_index);
	return user_fd_index; // 返回用户的文件描述符
}
