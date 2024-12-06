#include <stdio.h>
#include <string.h>
#include "filesys.h"

static struct Dinode block_buf[BLOCKSIZ / DINODESIZ]; // 存放i节点的临时数组
/*****************************************************
函数：ialloc
功能：分配磁盘i节点，返回相应的内存i节点指针
******************************************************/
struct INode *ialloc()
{
	struct INode *temp_inode;
	unsigned int cur_di;
	int i, count, block_end_flag;

	// I界点分配时从低位到高位使用，并且分配的i节点也是由低到高
	if (fileSystem.free_inode_pointer == NICINOD)
	{
		i = 0;
		block_end_flag = 1;
		count = fileSystem.free_inode_pointer = fileSystem.free_inode_count > NICINOD ? 0 : (NICINOD - fileSystem.free_inode_count);
		cur_di = fileSystem.last_allocated_inode;
		while (count < NICINOD)
		{ // 空闲i节点数组没有装满且磁盘中还有空闲i节点
			if (block_end_flag)
			{
				memcpy(block_buf, disk + DINODESTART + cur_di * DINODESIZ, BLOCKSIZ); // 从i节点去中读一个盘块到临时数组
				block_end_flag = 0;
				i = 0;
			}
			while (block_buf[IUPDATE].di_mode != DIEMPTY)
			{ // 临时数组为空，则读到空闲i节点数组中
				cur_di++;
				i++;
			}
			if (i == NICINOD)
			{ // 空闲i节点数组已满
				block_end_flag = 1;
				continue;
			}
			fileSystem.free_inodes[count++] = cur_di;
		}
		fileSystem.last_allocated_inode = cur_di; // 重新设铭记i节点
	}
	/*分配空闲i节点*/
	temp_inode = iget(fileSystem.free_inodes[fileSystem.free_inode_pointer]);
	memcpy(disk + DINODESTART + fileSystem.free_inodes[fileSystem.free_inode_pointer] * DINODESIZ,
		   &temp_inode->di_number, sizeof(struct Dinode));
	fileSystem.free_inode_pointer++;
	fileSystem.free_inode_count--;
	fileSystem.superblock_modified_flag = SUPDATE;
	return temp_inode;
}

void ifree(unsigned int dinodeid)
{
	fileSystem.free_inode_count--; // 空闲i节点数减一
	if (fileSystem.free_inode_pointer != 0)
	{ // 空闲i节点数组未满
		fileSystem.free_inode_pointer--;
		fileSystem.free_inodes[fileSystem.free_inode_pointer] = dinodeid;
	}
	else
	{
		if (dinodeid < fileSystem.last_allocated_inode)
		{
			// 新释放i节点号小于铭记i节点号，则丢弃原铭记i节点，设新的铭记i节点为新释放的铭记i节点
			fileSystem.free_inodes[NICINOD] = dinodeid;
			fileSystem.last_allocated_inode = dinodeid;
		}
	}
	return;
}
