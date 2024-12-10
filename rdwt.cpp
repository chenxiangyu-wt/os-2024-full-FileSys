#include <stdio.h>
#include <cstring>
#include "file_sys.hpp"
#include "globals.hpp"

unsigned int read(int fd, char *buf, unsigned int size)
{
	unsigned long off;
	unsigned int block, block_off, i, j;
	struct MemoryINode *inode;
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
	block_off = off % BLOCK_SIZE;
	block = off / BLOCK_SIZE;
	if (block_off + size < BLOCK_SIZE)
	{
		memcpy(buf, disk + DATA_START_POINTOR + inode->block_addresses[block] * BLOCK_SIZE + block_off, size);
		return size;
	}
	memcpy(temp_buf, disk + DATA_START_POINTOR + inode->block_addresses[block] * BLOCK_SIZE + block_off, BLOCK_SIZE - block_off);
	temp_buf += BLOCK_SIZE - block_off;
	j = (inode->file_size - off - block_off) / BLOCK_SIZE;
	for (i = 0; i < (size - (BLOCK_SIZE - block_off)) / BLOCK_SIZE; i++)
	{
		memcpy(temp_buf, disk + DATA_START_POINTOR + inode->block_addresses[j + i] * BLOCK_SIZE, BLOCK_SIZE);
		temp_buf += BLOCK_SIZE;
	}

	block_off = (size - (BLOCK_SIZE - block_off)) % BLOCK_SIZE;
	memcpy(temp_buf, disk + DATA_START_POINTOR + i * BLOCK_SIZE, block_off);
	sys_ofile[user[user_id].open_files[fd]].offset += size;
	return size;
}

unsigned int write(int fd, char *buf, unsigned int size)
{
	unsigned long off;
	unsigned int block, block_off, i, j;
	struct MemoryINode *inode;
	char *temp_buf;

	inode = sys_ofile[user[user_id].open_files[fd]].inode;
	if (!(sys_ofile[user[user_id].open_files[fd]].flag & FWRITE))
	{
		printf("\nthe file is not opened for write\n");
		return 0;
	}
	// add by liwen to check the filesize and alloc the BLOCK
	off = sys_ofile[user[user_id].open_files[fd]].offset;
	block = ((off + size) - inode->file_size) / BLOCK_SIZE; // ÉÐÐè¸öÊý
	if (((off + size) - inode->file_size) % BLOCK_SIZE)
		block++;
	if (fileSystem.free_block_count < block)
	{
		printf("Not enough space to write so much bytes!\n");
		return 0;
	}
	j = inode->file_size / BLOCK_SIZE;
	if (inode->file_size % BLOCK_SIZE)
	{
		j++;
	}
	if (j + block > ADDRESS_POINTOR_NUM)
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
	block_off = off % BLOCK_SIZE;
	block = off / BLOCK_SIZE;

	if (block_off + size < BLOCK_SIZE)
	{
		memcpy(disk + DATA_START_POINTOR + inode->block_addresses[block] * BLOCK_SIZE + block_off, buf, size);
		return size;
	}
	memcpy(disk + DATA_START_POINTOR + inode->block_addresses[block] * BLOCK_SIZE + block_off, temp_buf, BLOCK_SIZE - block_off);

	temp_buf += BLOCK_SIZE - block_off;
	for (i = 0; i < (size - (BLOCK_SIZE - block_off)) / BLOCK_SIZE; i++)
	{
		memcpy(disk + DATA_START_POINTOR + inode->block_addresses[block + 1 + i] * BLOCK_SIZE, temp_buf, BLOCK_SIZE);
		temp_buf += BLOCK_SIZE;
	}
	block_off = (size - (BLOCK_SIZE - block_off)) % BLOCK_SIZE;
	memcpy(disk + DATA_START_POINTOR + block * BLOCK_SIZE, temp_buf, block_off);
	sys_ofile[user[user_id].open_files[fd]].offset += size;
	return size;
}
