#include <stdio.h>
#include <string.h>
#include <iostream>
#include <stack>
#include <string>
#include "file_sys.hpp"
#include "globals.hpp"
#include "dEntry.hpp"
#include "iNode.hpp"

std::string get_current_path()
{
	std::stack<std::string> path_stack;
	MemoryINode *current_inode = cwd; // 当前目录的内存 i-node
	MemoryINode *parent_inode;
	bool is_root = false;

	while (!is_root)
	{
		if (current_inode->disk_inode_number == 1) // 根目录
		{
			is_root = true;
			break;
		}

		// 获取父目录的 inode
		parent_inode = get_parent_inode(current_inode);

		if (!parent_inode)
		{
			std::cerr << "Error: Unable to locate parent directory." << std::endl;
			return "/";
		}

		// 从父目录中找到当前目录的名称
		DirectoryEntry parent_entries[BLOCK_SIZE / sizeof(DirectoryEntry)];
		// std::cout << sizeof(parent_entries) << std::endl;
		memcpy(parent_entries, disk + DATA_START_POINTOR + parent_inode->block_addresses[0] * BLOCK_SIZE, sizeof(parent_entries));

		for (int i = 0; i < parent_inode->file_size / sizeof(DirectoryEntry); i++)
		{
			if (parent_entries[i].inode_number == current_inode->disk_inode_number)
			{
				path_stack.push(parent_entries[i].name); // 将目录名压栈
				break;
			}
		}

		// 更新为父目录
		current_inode = parent_inode;
	}

	// 拼接路径
	std::string full_path = "/";
	while (!path_stack.empty())
	{
		full_path += path_stack.top() + "/";
		path_stack.pop();
	}

	return full_path;
}
void _dir()
{
	uint32_t mode;					// 存储权限位
	struct MemoryINode *temp_inode; // 临时 i-node 指针

	// 打印当前目录信息
	// std::string cur_path = get_current_path(cwd);
	// std::cout << "当前目录：" << cur_path << std::endl;
	std::cout << "目录项数：" << dir.entry_count << std::endl;

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
	int entry_index = find_empty_entry();
	if (entry_index == -1)
	{
		printf("当前目录已满，无法创建目录！\n");
		return;
	}

	// 分配 inode
	MemoryINode *new_inode = ialloc();
	if (!new_inode)
	{
		printf("分配 inode 失败！\n");
		return;
	}

	// 设置当前目录的目录项
	strcpy(dir.entries[entry_index].name, dirname);
	dir.entries[entry_index].inode_number = new_inode->disk_inode_number;
	dir.entries[entry_index].type = DENTRY_DIR;
	dir.entry_count++;

	// 初始化新目录的目录项 (.. 和 .)
	DirectoryEntry new_dir_entries[2];
	strcpy(new_dir_entries[0].name, ".."); // 父目录
	new_dir_entries[0].inode_number = cwd->disk_inode_number;
	new_dir_entries[0].type = DENTRY_DIR;

	strcpy(new_dir_entries[1].name, "."); // 当前目录
	new_dir_entries[1].inode_number = new_inode->disk_inode_number;
	new_dir_entries[1].type = DENTRY_DIR;

	// 分配数据块并写入目录项
	uint32_t block = balloc();
	memcpy(disk + DATA_START_POINTOR + block * BLOCK_SIZE, new_dir_entries, sizeof(new_dir_entries));

	// 初始化新目录的 inode
	new_inode->file_size = sizeof(new_dir_entries);
	new_inode->mode = DIDIR | 0755;
	new_inode->block_addresses[0] = block;

	iput(new_inode);
	printf("目录 %s 创建成功！\n", dirname);
}

int chdir(const char *dirname)
{
	int dirid;
	MemoryINode *inode;
	uint16_t block;
	int j, low = 0, high = 0;

	dirid = namei(dirname, DENTRY_DIR);
	if (dirid == -1)
	{
		printf("不存在目录%s！\n", dirname);
		return 0;
	}
	inode = iget(dir.entries[dirid].inode_number);
	if (!(inode->mode & DIDIR))
	{
		printf("不是一个目录！\n");
		return 0;
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
	j = cwd->file_size % BLOCK_SIZE ? 1 : 0;
	for (uint16_t i = 0; i < cwd->file_size / BLOCK_SIZE + j; i++)
	{
		bfree(cwd->block_addresses[i]);
	}
	for (int i = 0; i < dir.entry_count; i += BLOCK_SIZE / (sizeof(DirectoryEntry)))
	{
		block = balloc();
		cwd->block_addresses[i] = block;
		memcpy(disk + DATA_START_POINTOR + block * BLOCK_SIZE, &dir.entries[i], BLOCK_SIZE);
	}
	cwd->file_size = dir.entry_count * (sizeof(DirectoryEntry));
	iput(cwd);
	cwd = inode;
	j = 0;

	for (uint16_t i = 0; i < inode->file_size / BLOCK_SIZE + 1; i++)
	{
		memcpy(&dir.entries[j], disk + DATA_START_POINTOR + inode->block_addresses[i] * BLOCK_SIZE, BLOCK_SIZE);
		j += BLOCK_SIZE / (sizeof(DirectoryEntry));
	}
	dir.entry_count = cwd->file_size / (sizeof(DirectoryEntry));
	for (int i = dir.entry_count; i < ENTRY_NUM; i++)
	{
		dir.entries[i].inode_number = 0;
	}
	// end by xiao
	std::cout << "当前inode_number：" << dir.entries[1].inode_number << std::endl;
	std::cout << "父目录inode_number： " << dir.entries[0].inode_number << std::endl;
	return 1;
}
