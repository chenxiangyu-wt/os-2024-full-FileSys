#include <stdio.h>
#include <string.h>
#include "filesys.h"

void install()
{

	/* 1. read the filsys from the superblock*/ // xiao
	memcpy(&fileSystem, disk + BLOCKSIZ, sizeof(FileSystem));

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
			user[i].open_files[j] = SYSOPENFILE + 1;
	}

	/*5. read the main directory to initialize the dir*/
	cur_path_inode = iget(1);
	dir.entry_count = cur_path_inode->file_size / (ENTRYNUM + 4); // xiao 2-->4

	for (unsigned int i = 0; i < ENTRYNAMELEN; i++)
	{
		strcpy(dir.entries[i].name, "             ");
		dir.entries[i].inode_number = 0;
	}
	unsigned int i;
	for (i = 0; i < dir.entry_count / (BLOCKSIZ / (ENTRYNUM + 4)); i++)
	{
		memcpy(&dir.entries[(BLOCKSIZ / (ENTRYNUM + 4)) * i],
			   disk + DATASTART + BLOCKSIZ * cur_path_inode->block_addresses[i], DINODESIZ);
	}

	return;
}
