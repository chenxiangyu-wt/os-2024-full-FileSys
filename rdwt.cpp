#include <stdio.h>
#include <cstring>
#include "filesys.h"

unsigned int read(int fd, char *buf, unsigned int size)
{
	unsigned long off;
	unsigned int block, block_off, i, j;
	struct INode *inode;
	char *temp_buf;

	inode = sys_ofile[user[user_id].open_files[fd]].inode;
	if (!(sys_ofile[user[user_id].open_files[fd]].flag & FREAD))
	{
		printf("\nthe file is not opened for read\n");
		return 0;
	}
	temp_buf = buf;
	off = sys_ofile[user[user_id].open_files[fd]].offset;
	if ((off + size) > inode->file_size)
	{
		size = inode->file_size - off;
	}
	block_off = off % BLOCKSIZ;
	block = off / BLOCKSIZ;
	if (block_off + size < BLOCKSIZ)
	{
		memcpy(buf, disk + DATASTART + inode->block_addresses[block] * BLOCKSIZ + block_off, size);
		return size;
	}
	memcpy(temp_buf, disk + DATASTART + inode->block_addresses[block] * BLOCKSIZ + block_off, BLOCKSIZ - block_off);
	temp_buf += BLOCKSIZ - block_off;
	j = (inode->file_size - off - block_off) / BLOCKSIZ;
	for (i = 0; i < (size - (BLOCKSIZ - block_off)) / BLOCKSIZ; i++)
	{
		memcpy(temp_buf, disk + DATASTART + inode->block_addresses[j + i] * BLOCKSIZ, BLOCKSIZ);
		temp_buf += BLOCKSIZ;
	}

	block_off = (size - (BLOCKSIZ - block_off)) % BLOCKSIZ;
	memcpy(temp_buf, disk + DATASTART + i * BLOCKSIZ, block_off);
	sys_ofile[user[user_id].open_files[fd]].offset += size;
	return size;
}

unsigned int write(int fd, char *buf, unsigned int size)
{
	unsigned long off;
	unsigned int block, block_off, i, j;
	struct INode *inode;
	char *temp_buf;

	inode = sys_ofile[user[user_id].open_files[fd]].inode;
	if (!(sys_ofile[user[user_id].open_files[fd]].flag & FWRITE))
	{
		printf("\nthe file is not opened for write\n");
		return 0;
	}
	// add by liwen to check the filesize and alloc the BLOCK
	off = sys_ofile[user[user_id].open_files[fd]].offset;
	block = ((off + size) - inode->file_size) / BLOCKSIZ; // ÉÐÐè¸öÊý
	if (((off + size) - inode->file_size) % BLOCKSIZ)
		block++;
	if (fileSystem.free_block_count < block)
	{
		printf("Not enough space to write so much bytes!\n");
		return 0;
	}
	j = inode->file_size / BLOCKSIZ;
	if (inode->file_size % BLOCKSIZ)
	{
		j++;
	}
	if (j + block > NADDR)
	{
		printf("To write so much bytes will exceed the file limit!!\n");
		return 0;
	}
	for (i = j; i < j + block; i++)
	{
		inode->block_addresses[i] = balloc();
	}
	inode->file_size += size;
	// end add
	temp_buf = buf;

	off = sys_ofile[user[user_id].open_files[fd]].offset;
	block_off = off % BLOCKSIZ;
	block = off / BLOCKSIZ;

	if (block_off + size < BLOCKSIZ)
	{
		memcpy(disk + DATASTART + inode->block_addresses[block] * BLOCKSIZ + block_off, buf, size);
		return size;
	}
	memcpy(disk + DATASTART + inode->block_addresses[block] * BLOCKSIZ + block_off, temp_buf, BLOCKSIZ - block_off);

	temp_buf += BLOCKSIZ - block_off;
	for (i = 0; i < (size - (BLOCKSIZ - block_off)) / BLOCKSIZ; i++)
	{
		memcpy(disk + DATASTART + inode->block_addresses[block + 1 + i] * BLOCKSIZ, temp_buf, BLOCKSIZ);
		temp_buf += BLOCKSIZ;
	}
	block_off = (size - (BLOCKSIZ - block_off)) % BLOCKSIZ;
	memcpy(disk + DATASTART + block * BLOCKSIZ, temp_buf, block_off);
	sys_ofile[user[user_id].open_files[fd]].offset += size;
	return size;
}
