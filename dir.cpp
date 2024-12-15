#include <stdio.h>
#include <string.h>
#include <iostream>
#include "file_sys.hpp"
#include "globals.hpp"

void _dir()
{
	uint32_t mode;					// 存储权限位
	struct MemoryINode *temp_inode; // 临时 i-node 指针

	// 打印当前目录信息
	printf("\nCURRENT DIRECTORY: %s\n", dir.entries[0].name);
	printf("当前共有 %d 个文件/目录\n", dir.entry_count);

	// 遍历当前目录的所有有效项
	for (int i = 0; i < dir.entry_count; i++) // 使用 entry_count 遍历
	{
		if (dir.entries[i].inode_number != DIEMPTY) // 跳过空项
		{
			printf("%-14s", dir.entries[i].name);			// 打印文件/目录名称
			temp_inode = iget(dir.entries[i].inode_number); // 获取对应的 inode

			// 打印权限
			mode = temp_inode->mode & 0x1FF; // 只取 9 位权限位
			for (int j = 8; j >= 0; j--)	 // 从高位到低位打印权限
			{
				char perm = (mode & (1 << j)) ? "rwx"[(j % 3)] : '-';
				printf("%c", perm);
			}

			printf("\ti_ino->%d\t", dir.entries[i].inode_number);

			// 文件类型判断
			if (temp_inode->mode & DIFILE) // 是文件
			{
				printf(" %d ", temp_inode->file_size); // 打印文件大小
				printf("block chain:");
				uint32_t block_count = (temp_inode->file_size + BLOCK_SIZE - 1) / BLOCK_SIZE; // 计算块数
				for (uint32_t k = 0; k < block_count; k++)
				{
					printf(" %4d", temp_inode->block_addresses[k]); // 打印数据块号
				}
				printf("\n");
			}
			else // 是目录
			{
				printf("<dir>\n");
			}

			iput(temp_inode); // 释放内存中的 inode
		}
	}
}

void mkdir(const char *dirname)
{
	int found_inode_id, free_dir_entry_index;
	MemoryINode *inode;
	DirectoryEntry buf[BLOCK_SIZE / (sizeof(DirectoryEntry))];
	uint32_t block;

	found_inode_id = namei(dirname, DENTRY_DIR);
	if (found_inode_id != -1)
	{
		inode = iget(found_inode_id);
		if (inode->mode & DIDIR)
			printf("目录%s已存在！\n", dirname); // xiao
		else
			printf("%s是一个文件！\n", dirname);
		iput(inode);
		return;
	}
	free_dir_entry_index = iname(dirname);								 // 取得在addr中的空闲项位置,并将目录名写到此项里
	inode = ialloc();													 // 分配i节点
	dir.entries[free_dir_entry_index].inode_number = inode->status_flag; // 设置该目录的磁盘i节点号
	dir.entry_count++;													 // 目录数++

	strcpy(buf[0].name, ".."); // 子目录的上一层目录 当前目录
	buf[0].inode_number = cur_path_inode->status_flag;
	buf[0].type = DENTRY_DIR;
	strcpy(buf[1].name, ".");
	buf[1].inode_number = inode->status_flag; // 子目录的本目录 子目录
	buf[1].type = DENTRY_DIR;
	dir.entries[free_dir_entry_index].type = DENTRY_DIR;

	block = balloc();
	memcpy(disk + DATA_START_POINTOR + block * BLOCK_SIZE, buf, BLOCK_SIZE);

	inode->file_size = 2 * (sizeof(DirectoryEntry));
	inode->reference_count = 1;
	inode->mode = user[user_id].default_mode | DIDIR;
	inode->owner_uid = user[user_id].user_id;
	inode->owner_gid = user[user_id].group_id;
	inode->block_addresses[0] = block;

	iput(inode);
	return;
}

void chdir(const char *dirname)
{
	int dirid;
	struct MemoryINode *inode;
	uint16_t block;
	int j, low = 0, high = 0;

	dirid = namei(dirname, DENTRY_DIR);
	if (dirid == -1)
	{
		printf("不存在目录%s！\n", dirname);
		return;
	}
	inode = iget(dir.entries[dirid].inode_number);
	if (!(inode->mode & DIDIR))
	{
		printf("不是一个目录！\n");
		return;
	}
	for (int i = 0; i < dir.entry_count; i++)
	{
		if (dir.entries[i].inode_number == 0)
		{
			for (j = ENTRY_NUM - 1; j >= 0 && dir.entries[j].inode_number == 0; j--)
				;
			memcpy(&dir.entries[i], &dir.entries[j], sizeof(DirectoryEntry)); // xiao
			dir.entries[j].inode_number = 0;
		}
	}
	j = cur_path_inode->file_size % BLOCK_SIZE ? 1 : 0;
	for (uint16_t i = 0; i < cur_path_inode->file_size / BLOCK_SIZE + j; i++)
	{
		bfree(cur_path_inode->block_addresses[i]);
	}
	for (int i = 0; i < dir.entry_count; i += BLOCK_SIZE / (sizeof(DirectoryEntry)))
	{
		block = balloc();
		cur_path_inode->block_addresses[i] = block;
		memcpy(disk + DATA_START_POINTOR + block * BLOCK_SIZE, &dir.entries[i], BLOCK_SIZE);
	}
	cur_path_inode->file_size = dir.entry_count * (sizeof(DirectoryEntry));
	iput(cur_path_inode);
	cur_path_inode = inode;

	j = 0;
	for (uint16_t i = 0; i < inode->file_size / BLOCK_SIZE + 1; i++)
	{
		memcpy(&dir.entries[j], disk + DATA_START_POINTOR + inode->block_addresses[i] * BLOCK_SIZE, BLOCK_SIZE);
		j += BLOCK_SIZE / (sizeof(DirectoryEntry));
	}
	dir.entry_count = cur_path_inode->file_size / (sizeof(DirectoryEntry));
	for (int i = dir.entry_count; i < ENTRY_NUM; i++)
	{
		dir.entries[i].inode_number = 0;
	}

	// end by xiao

	return;
}
