#include <stdio.h>
#include <string.h>
#include "file_sys.hpp"
#include "globals.hpp"
#include "dEntry.hpp"
/*********************************************************************
函数：creat
功能：创建文件，存在且可写则覆盖，否则申请i节点，并打开该文件，返回文件指针
**********************************************************************/
int creat(uint32_t user_id, const char *filename, uint16_t mode)
{
	struct MemoryINode *inode;
	int entry_index, empty_entry_index;
	uint32_t i, j;

	// 1. 查找目录项，查看是否存在同名文件或目录
	entry_index = namei(filename, DENTRY_FILE | DENTRY_DIR);
	if (entry_index != -1)
	{
		inode = iget(dir.entries[entry_index].inode_number);
		if (inode == NULL)
		{
			printf("Error: Failed to load inode.\n");
			return -1;
		}

		if (inode->mode & DIDIR) // 如果是目录，不能覆盖
		{
			printf("存在同名目录，无法创建文件！\n");
			iput(inode);
			return -1;
		}

		// 2. 覆盖同名文件内容
		if (access(user_id, inode, WRITE) == 0)
		{
			printf("\ncreat access not allowed: no write permission!\n");
			iput(inode);
			return -1;
		}

		printf("同名文件已存在，清空内容并覆盖...\n");

		// 清空文件内容，回收数据块
		j = inode->file_size % BLOCK_SIZE ? 1 : 0;
		for (i = 0; i < inode->file_size / BLOCK_SIZE + j; i++)
		{
			bfree(inode->block_addresses[i]);
		}

		inode->file_size = 0;

		// 重置已打开文件的偏移量
		for (i = 0; i < SYSTEM_MAX_OPEN_FILE_NUM; i++)
		{
			if (system_opened_file[i].reference_count != 0 && system_opened_file[i].inode == inode)
			{
				system_opened_file[i].offset = 0;
			}
		}

		iput(inode);
		return open(user_id, filename, WRITE);
	}

	// 3. 不存在同名文件，分配新 inode
	printf("创建新文件: %s\n", filename);

	inode = ialloc();
	if (!inode)
	{
		printf("Error: Failed to allocate inode.\n");
		return -1;
	}

	empty_entry_index = iname(filename);
	if (empty_entry_index == -1)
	{
		printf("Error: Directory is full, cannot create more files.\n");
		iput(inode);
		return -1;
	}

	strcpy(dir.entries[empty_entry_index].name, filename);
	dir.entries[empty_entry_index].inode_number = inode->disk_inode_number;
	dir.entries[empty_entry_index].type = DENTRY_FILE;
	dir.entry_count++;

	inode->mode = mode | DIFILE;
	inode->owner_uid = user[user_id].user_id;
	inode->owner_gid = user[user_id].group_id;
	inode->file_size = 0;
	inode->reference_count = 1;

	iput(inode);
	return open(user_id, filename, WRITE);
}

int _creat(uint32_t user_id, const char *filename, uint16_t mode)
{
	struct MemoryINode *inode;
	int entry_index, empty_entry_index;

	// 1. 查找目录项，查看是否存在同名文件或目录
	entry_index = namei(filename, DENTRY_FILE | DENTRY_DIR);
	if (entry_index != -1)
	{
		inode = iget(dir.entries[entry_index].inode_number);
		if (inode == NULL)
		{
			printf("Error: Failed to load inode.\n");
			return -1;
		}

		if (inode->mode & DIDIR)
		{
			printf("存在同名目录，无法创建文件！\n");
		}
		else
		{
			printf("Error: 文件 %s 已存在，无法重复创建！\n", filename);
		}

		iput(inode);
		return -1;
	}

	// 2. 不存在同名文件，分配新 inode
	printf("创建新文件: %s\n", filename);

	inode = ialloc();
	if (!inode)
	{
		printf("Error: Failed to allocate inode.\n");
		return -1;
	}

	empty_entry_index = iname(filename);
	if (empty_entry_index == -1)
	{
		printf("Error: Directory is full, cannot create more files.\n");
		iput(inode);
		return -1;
	}

	strcpy(dir.entries[empty_entry_index].name, filename);
	dir.entries[empty_entry_index].inode_number = inode->disk_inode_number;
	dir.entries[empty_entry_index].type = DENTRY_FILE;
	dir.entry_count++;

	inode->mode = mode | DIFILE;
	inode->owner_uid = user[user_id].user_id;
	inode->owner_gid = user[user_id].group_id;
	inode->file_size = 0;
	inode->reference_count = 1;

	iput(inode);
	return open(user_id, filename, WRITE);
}
