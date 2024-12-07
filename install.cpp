#include <stdio.h>
#include <string.h>
#include <iostream>
#include "filesys.hpp"

void install()
{
	std::cout << "installing" << std::endl;
	std::cout << "sizeof(FileSystem):" << sizeof(FileSystem) << std::endl;
	std::cout << "sizeof(Directory):" << sizeof(Directory) << std::endl;
	/* 1. read the filsys from the superblock*/ // xiao
	memcpy(&fileSystem, disk + BLOCK_SIZE, sizeof(FileSystem));

	/* 2. initialize the inode hash chain*/
	for (unsigned int i = 0; i < NHINO; i++)
	{
		hinode[i].prev_inode = NULL;
	}

	/*3. initialize the sys_ofile*/
	for (unsigned int i = 0; i < SYSOPENFILE; i++)
	{
		sys_ofile[i].reference_count = 0;
		sys_ofile[i].inode = NULL;
	}

	/*4. initialize the user*/
	for (unsigned int i = 0; i < USERNUM; i++)
	{
		user[i].user_id = 0;
		user[i].group_id = 0;
		for (unsigned int j = 0; j < NOFILE; j++)
		{
			user[i].open_files[j] = SYSOPENFILE + 1;
		}
	}

	/*5. read the root directory to initialize the dir*/
	cur_path_inode = iget(1);
	std::cout << "cur_path_inode->file_size:" << cur_path_inode->file_size << std::endl;
	dir.entry_count = cur_path_inode->file_size / (sizeof(DirectoryEntry)); // xiao 2-->4
	std::cout << "dir.entry_count:" << dir.entry_count << std::endl;
	for (unsigned int i = 0; i < ENTRY_NAME_LEN; i++)
	{
		strcpy(dir.entries[i].name, "             ");
		dir.entries[i].inode_number = 0;
	}
	std::cout << "dir.entry_count:" << dir.entry_count << std::endl;
	std::cout << "dir.entry_count / (BLOCKSIZ / (sizeof(DirectoryEntry)))" << dir.entry_count / (BLOCK_SIZE / (sizeof(DirectoryEntry))) << std::endl;
	for (unsigned int i = 0; i < dir.entry_count / (BLOCK_SIZE / (sizeof(DirectoryEntry))); i++)
	{
		std::cout << "偏移量:" << DATA_START_POINTOR + BLOCK_SIZE * cur_path_inode->block_addresses[i] << std::endl;
		memcpy(&dir.entries[(BLOCK_SIZE / (sizeof(DirectoryEntry))) * i],
			   disk + DATA_START_POINTOR + BLOCK_SIZE * cur_path_inode->block_addresses[i], DISK_INODE_SIZE);
	}
	std::cout << "installed" << std::endl;
	return;
}
