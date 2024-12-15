#include <stdio.h>
#include "file_sys.hpp"
#include "globals.hpp"

short open(int user_id, const char *filename, char openmode)
{
	int disk_inode_id;
	struct MemoryINode *inode;
	uint32_t i, j, k;

	disk_inode_id = namei(filename, DENTRY_FILE);
	if (disk_inode_id == 0)
	{
		printf("\nfile does not existed!!!\n");
		return -1;
	}
	inode = iget(dir.entries[disk_inode_id].inode_number);
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
		if (system_opened_file[i].reference_count == 0)
			break;
	}

	if (i == SYSOPENFILE)
	{
		printf("\nsystem open file too much\n");
		iput(inode);
		return -1;
	}

	system_opened_file[i].inode = inode;
	system_opened_file[i].flag = openmode;
	system_opened_file[i].reference_count = 1;

	if (openmode & FAPPEND)
	{
		system_opened_file[i].offset = inode->file_size;
	}
	else
	{
		system_opened_file[i].offset = 0;
	}

	for (j = 0; j < NOFILE; j++)
	{
		if (user[user_id].open_files[j] == SYSOPENFILE + 1)
			break;
	}

	if (j == NOFILE)
	{
		printf("\nuser open file too much!!!\n");
		system_opened_file[i].reference_count = 0;
		iput(inode);
		return -1;
	}

	user[user_id].open_files[j] = i;

	if (openmode & FWRITE)
	{
		k = inode->file_size % BLOCK_SIZE ? 1 : 0;
		for (i = 0; i < inode->file_size / BLOCK_SIZE + k; i++)
			bfree(inode->block_addresses[i]);
		inode->file_size = 0;
	}
	return j;
}
