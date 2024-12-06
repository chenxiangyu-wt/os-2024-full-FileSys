#include <stdio.h>
#include <string.h>
#include "filesys.h"

void install()
{
	/* 定义常量 */
	const size_t DirectoryEntrySize = ENTRYNUM + 4;				  // 目录项的大小
	const size_t EntriesPerBlock = BLOCKSIZ / DirectoryEntrySize; // 每个块的目录项数量

	/* 1. 从超级块读取文件系统信息 */
	memcpy(&fileSystem, disk + BLOCKSIZ, sizeof(FileSystem));

	/* 2. 初始化 i-node 哈希链表 */
	for (unsigned int i = 0; i < NHINO; i++)
	{
		hinode[i].prev_inode = NULL;
	}

	/* 3. 初始化系统打开文件表 */
	for (unsigned int i = 0; i < SYSOPENFILE; i++)
	{
		sys_ofile[i].reference_count = 0;
		sys_ofile[i].inode = NULL;
	}

	/* 4. 初始化用户表 */
	for (unsigned int i = 0; i < USERNUM; i++)
	{
		user[i].user_id = 0;
		user[i].group_id = 0;
		for (unsigned int j = 0; j < NOFILE; j++)
		{
			user[i].open_files[j] = SYSOPENFILE + 1;
		}
	}

	/* 5. 从主目录读取目录信息，初始化当前目录 */
	cur_path_inode = iget(1);
	dir.entry_count = cur_path_inode->file_size / DirectoryEntrySize;

	/* 初始化目录项数组 */
	for (unsigned int i = 0; i < ENTRYNAMELEN; i++)
	{
		strcpy(dir.entries[i].name, "             ");
		dir.entries[i].inode_number = 0;
	}

	/* 遍历并加载当前目录的所有目录项 */
	unsigned int i;
	for (i = 0; i < dir.entry_count / EntriesPerBlock; i++)
	{
		memcpy(
			&dir.entries[EntriesPerBlock * i],
			disk + DATASTART + BLOCKSIZ * cur_path_inode->block_addresses[i],
			BLOCKSIZ);
	}

	/* 加载剩余的目录项 */
	memcpy(
		&dir.entries[EntriesPerBlock * i],
		disk + DATASTART + BLOCKSIZ * cur_path_inode->block_addresses[i],
		BLOCKSIZ);
}
