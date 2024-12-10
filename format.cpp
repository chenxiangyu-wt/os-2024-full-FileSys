#include <cstring>
#include <iostream>
#include "globals.hpp"

void format()
{
	MemoryINode *inode;
	uint32_t PASSWORD_FILE_LIMITE = 32;
	DirectoryEntry dir_buf[BLOCK_SIZE / sizeof(DirectoryEntry)];
	UserPassword passwd[PASSWORD_FILE_LIMITE];
	uint32_t block_buf[BLOCK_SIZE / sizeof(int)];

	// 1. 初始化磁盘
	memset(disk, 0, DISK_SIZE); // 将整个磁盘清零

	// 2. 初始化密码文件内容
	memset(passwd, 0, sizeof(passwd));
	passwd[0] = {2116, 3, "dddd"};
	passwd[1] = {2117, 3, "bbbb"};
	passwd[2] = {2118, 4, "abcd"};
	passwd[3] = {2119, 4, "cccc"};
	passwd[4] = {2120, 5, "eeee"};

	for (int i = 5; i < PASSWORD_FILE_LIMITE; i++)
	{
		passwd[i].user_id = 0;
		passwd[i].group_id = 0;
		memset(passwd[i].password, ' ', PWDSIZ);
	}

	// 3. 初始化超级块 (FileSystem)
	fileSystem.inode_block_count = DISK_INODE_AREA_SIZE;
	fileSystem.data_block_count = DATA_BLOCK_AREA_SIZE;

	fileSystem.free_inode_count = (DISK_INODE_AREA_SIZE * BLOCK_SIZE) / DISK_INODE_SIZE - 4; // 前 4 个已使用
	fileSystem.free_block_count = DATA_BLOCK_AREA_SIZE - 3;									 // 数据区保留了 3 块

	// 初始化空闲 i-node 数组
	for (int i = 0; i < NICINOD; i++)
	{
		fileSystem.free_inodes[i] = i + 4; // 从 i-node 4 开始
	}
	fileSystem.free_inode_pointer = 0;
	fileSystem.last_allocated_inode = NICINOD + 4;

	// 初始化空闲块栈
	int block_index = DATA_BLOCK_AREA_SIZE - 1;
	for (int i = 0; i < NICFREE; i++)
	{
		fileSystem.free_blocks[i] = block_index--;
	}
	fileSystem.free_block_pointer = 0;

	// 4. 创建根目录 (inode 1)
	inode = iget(1);
	inode->mode = DIDIR | 0755; // 目录模式
	inode->file_size = 3 * sizeof(DirectoryEntry);
	inode->reference_count = 1;
	inode->block_addresses[0] = balloc();

	memset(dir_buf, 0, sizeof(dir_buf));
	strcpy(dir_buf[0].name, "..");
	dir_buf[0].inode_number = 1;
	strcpy(dir_buf[1].name, ".");
	dir_buf[1].inode_number = 1;
	strcpy(dir_buf[2].name, "etc");
	dir_buf[2].inode_number = 2;

	memcpy(disk + DATA_START_POINTOR + inode->block_addresses[0] * BLOCK_SIZE, dir_buf, BLOCK_SIZE);
	iput(inode);

	// 5. 创建 `etc` 目录 (inode 2)
	inode = iget(2);
	inode->mode = DIDIR | 0755;
	inode->file_size = 3 * sizeof(DirectoryEntry);
	inode->reference_count = 1;
	inode->block_addresses[0] = balloc();

	memset(dir_buf, 0, sizeof(dir_buf));
	strcpy(dir_buf[0].name, "..");
	dir_buf[0].inode_number = 1;
	strcpy(dir_buf[1].name, ".");
	dir_buf[1].inode_number = 2;
	strcpy(dir_buf[2].name, "password");
	dir_buf[2].inode_number = 3;

	memcpy(disk + DATA_START_POINTOR + inode->block_addresses[0] * BLOCK_SIZE, dir_buf, BLOCK_SIZE);
	iput(inode);

	// 6. 创建 `password` 文件 (inode 3)
	inode = iget(3);
	inode->mode = DIFILE | 0644; // 文件模式
	inode->file_size = BLOCK_SIZE;
	inode->reference_count = 1;
	inode->block_addresses[0] = balloc();

	memcpy(disk + DATA_START_POINTOR + inode->block_addresses[0] * BLOCK_SIZE, passwd, BLOCK_SIZE);
	iput(inode);

	// 7. 将超级块写入磁盘
	memcpy(disk + BLOCK_SIZE, &fileSystem, sizeof(FileSystem));

	std::cout << "Format completed. File system initialized." << std::endl;
}