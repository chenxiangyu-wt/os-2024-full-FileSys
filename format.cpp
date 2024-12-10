#include <stdio.h>
#include <string.h>
#include <iostream>
#include "file_sys.hpp"
#include "globals.hpp"

void initialize_password_file()
{
	UserPassword passwd[32];
	passwd[0].user_id = 2116;
	passwd[0].group_id = 03;
	strcpy(passwd[0].password, "dddd");

	passwd[1].user_id = 2117;
	passwd[1].group_id = 03;
	strcpy(passwd[1].password, "bbbb");

	passwd[2].user_id = 2118;
	passwd[2].group_id = 04;
	strcpy(passwd[2].password, "abcd");

	passwd[3].user_id = 2119;
	passwd[3].group_id = 04;
	strcpy(passwd[3].password, "cccc");

	passwd[4].user_id = 2120;
	passwd[4].group_id = 05;
	strcpy(passwd[4].password, "eeee");

	for (int i = 5; i < PWDNUM; i++)
	{
		passwd[i].user_id = 0;
		passwd[i].group_id = 0;
		strcpy(passwd[i].password, "            "); // PWDSIZ " "
	}

	memcpy(pwd, passwd, 32 * sizeof(struct UserPassword));
	memcpy(disk + DATA_START_POINTOR + BLOCK_SIZE * 2, passwd, BLOCK_SIZE);
}
void initialize_root_directory()
{
	DirectoryEntry dir_buf[BLOCK_SIZE / (sizeof(DirectoryEntry))];
	strcpy(dir_buf[0].name, "..");
	dir_buf[0].inode_number = 1;
	strcpy(dir_buf[1].name, ".");
	dir_buf[1].inode_number = 1;
	strcpy(dir_buf[2].name, "etc");
	dir_buf[2].inode_number = 2;
	memcpy(disk + DATA_START_POINTOR, &dir_buf, 3 * (sizeof(DirectoryEntry)));
}
void initialize_superblock()
{

	fileSystem.inode_block_count = DISK_INODE_AREA_SIZE;
	fileSystem.data_block_count = DATA_BLOCK_AREA_SIZE;

	fileSystem.free_inode_count = DISK_INODE_AREA_SIZE * BLOCK_SIZE / DISK_INODE_SIZE - 4;
	fileSystem.free_block_count = DATA_BLOCK_AREA_SIZE - 3;

	for (int i = 0; i < NICINOD; i++)
	{
		/* begin with 4,    0,1,2,3, is used by main,etc,password */
		fileSystem.free_inodes[i] = 4 + i;
	}

	fileSystem.free_inode_pointer = 0;
	fileSystem.last_allocated_inode = NICINOD + 4;

	unsigned int block_buf[BLOCK_SIZE / sizeof(int)];
	block_buf[NICFREE - 1] = DATA_BLOCK_AREA_SIZE + 1; /*FILEBLK+1 is a flag of end*/
	for (int i = 0; i < NICFREE - 1; i++)
	{
		block_buf[NICFREE - 2 - i] = DATA_BLOCK_AREA_SIZE - i - 1; // 从最后一个数据块开始分配??????
	}

	memcpy(disk + DATA_START_POINTOR + BLOCK_SIZE * (DATA_BLOCK_AREA_SIZE - NICFREE), block_buf, BLOCK_SIZE);
	int i, j;
	for (i = DATA_BLOCK_AREA_SIZE - 2 * NICFREE + 1; i > 2; i -= NICFREE)
	{
		for (j = 0; j < NICFREE; j++)
		{
			block_buf[j] = i + j;
		}
		memcpy(disk + DATA_START_POINTOR + BLOCK_SIZE * (i - 1), block_buf, BLOCK_SIZE);
	}
	i += NICFREE;
	j = 1;
	for (; i > 3; i--)
	{
		fileSystem.free_blocks[NICFREE - j] = i - 1;
		j++;
	}

	fileSystem.free_block_pointer = NICFREE - j + 1;
	memcpy(disk + BLOCK_SIZE, &fileSystem, sizeof(FileSystem));
}

void format()
{
	initialize_superblock();
	initialize_password_file();
	initialize_root_directory();
}