#include <stdio.h>
#include <cstring>
#include "filesys.hpp"

static unsigned int block_buf[BLOCK_SIZE];
/**********************************************************
函数：balloc
功能：维护超级块中的空闲数据栈，分配空闲数据块，并返回其块号
***********************************************************/
unsigned int balloc()
{
	unsigned int free_block;
	int i;

	// 如果没有空闲盘块
	if (fileSystem.free_block_count == 0)
	{
		printf("\nDisk Full!!!\n");
		return DISKFULL;
	}
	free_block = fileSystem.free_blocks[fileSystem.free_block_pointer]; // 取堆栈中的盘块号
	if (fileSystem.free_block_pointer == NICFREE - 1)
	{ // 如果堆栈只剩一个块
		memcpy(block_buf, disk + DATA_START_POINTOR + (free_block)*BLOCK_SIZE, BLOCK_SIZE);
		// 从中读取下一组块号
		for (i = 0; i < NICFREE; i++)
			fileSystem.free_blocks[i] = block_buf[i];
		fileSystem.free_block_pointer = 0; // 设置堆栈指针
	}
	else
	{									 // 如果堆栈中大于一个盘块
		fileSystem.free_block_pointer++; // 修改堆栈指针
	}
	fileSystem.free_block_count--; // 修改总块数
	fileSystem.superblock_modified_flag = SUPDATE;
	return free_block;
}

void bfree(unsigned int block_num)
{
	int i;

	if (fileSystem.free_block_pointer == 0)
	{ // 如果堆栈已满
		/*将当前堆栈内块号写入当前块号*/
		for (i = 0; i < NICFREE; i++)
			block_buf[i] = fileSystem.free_blocks[NICFREE - 1 - i];
		memcpy(disk + DATA_START_POINTOR + block_num * BLOCK_SIZE, block_buf, BLOCK_SIZE);
		fileSystem.free_block_pointer = NICFREE; // 清空堆栈
	}
	/*修改堆栈指针，并将当前块号压入堆栈*/
	fileSystem.free_block_pointer--;
	fileSystem.free_block_count++;
	fileSystem.free_blocks[fileSystem.free_block_pointer] = block_num;
	fileSystem.superblock_modified_flag = SUPDATE;
	return;
}
