#include <stdio.h>
#include "filesys.h"

short open(int user_id, const char *filename, char openmode)
{
	unsigned int dinodeid;
	struct MemoryINode *inode;
	uint32_t i, j, k;

	dinodeid = namei(filename);
	if (dinodeid == 0)
	{
		printf("\nfile does not existed!!!\n");
		return -1;
	}
	inode = iget(dir.entries[dinodeid].inode_number);
	if (!(inode->mode & DIFILE))
	{
		printf("%s is not a file!!!\n", filename);
		iput(inode);
		return -1;
	}
	if (!access(user_id, inode, openmode))
	{
		printf("\nfile open has not access!!!\n");
		iput(inode);
		return -1;
	}

	for (i = 1; i < SYSOPENFILE; i++)
	{
		if (sys_ofile[i].reference_count == 0)
			break;
	}

	if (i == SYSOPENFILE)
	{
		printf("\nsystem open file too much\n");
		iput(inode);
		return -1;
	}

	sys_ofile[i].inode = inode;
	sys_ofile[i].flag = openmode;
	sys_ofile[i].reference_count = 1;

	if (openmode & FAPPEND)
	{
		sys_ofile[i].offset = inode->file_size;
	}
	else
	{
		sys_ofile[i].offset = 0;
	}

	for (j = 0; j < NOFILE; j++)
	{
		if (user[user_id].open_files[j] == SYSOPENFILE + 1)
			break;
	}

	if (j == NOFILE)
	{
		printf("\nuser open file too much!!!\n");
		sys_ofile[i].reference_count = 0;
		iput(inode);
		return -1;
	}

	user[user_id].open_files[j] = i;

	if (openmode & FWRITE)
	{
		k = inode->file_size % BLOCKSIZ ? 1 : 0;
		for (i = 0; i < inode->file_size / BLOCKSIZ + k; i++)
			bfree(inode->block_addresses[i]);
		inode->file_size = 0;
	}
	return j;
}
