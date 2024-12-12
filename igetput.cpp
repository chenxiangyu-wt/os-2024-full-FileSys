#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_sys.hpp"
#include "globals.hpp"

/* 函数名：	iget 							*/
/* 作用：	为磁盘i结点分配对应的内存i结点	*/
/* 参数:	待分配的磁盘i结点号				*/
/* 返回值：	指向对应的内存i结点的指针		*/
MemoryINode *iget(uint32_t dinodeid)
{
	int inodeid = dinodeid % NHINO; // 计算哈希链表索引
	MemoryINode *temp = hinode[inodeid].prev_inode;

	// 1. 检查哈希链表中是否已存在该 i-node
	while (temp)
	{
		if (temp->status_flag == dinodeid) // 已存在
		{
			temp->reference_count++;
			return temp;
		}
		temp = temp->next;
	}

	// 2. 若没有找到，从磁盘加载 i-node
	long addr = DISK_INODE_START_POINTOR + dinodeid * DISK_INODE_SIZE;
	DiskINode d_inode;

	memcpy(&d_inode, disk + addr, sizeof(DiskINode)); // 读取磁盘 i-node

	// 3. 分配内存 i-node 并初始化
	MemoryINode *newinode = (MemoryINode *)malloc(sizeof(MemoryINode));
	newinode->reference_count = 1;
	newinode->status_flag = dinodeid;
	newinode->disk_inode_number = dinodeid;

	newinode->mode = d_inode.mode;
	newinode->owner_uid = d_inode.owner_uid;
	newinode->owner_gid = d_inode.owner_gid;
	newinode->file_size = d_inode.file_size;
	memcpy(newinode->block_addresses, d_inode.block_addresses, sizeof(d_inode.block_addresses));

	// 4. 插入哈希链表
	newinode->next = hinode[inodeid].prev_inode;
	newinode->prev = nullptr;

	if (hinode[inodeid].prev_inode)
		hinode[inodeid].prev_inode->prev = newinode;

	hinode[inodeid].prev_inode = newinode;

	return newinode;
}

/* 函数名：	iput							*/
/* 作用：	回收内存i结点					*/
/* 参数:	指向待回收的内存i结点指针		*/
/* 返回值：	无								*/
void iput(struct MemoryINode *pinode)
{
	long addr;

	// 1. 如果引用计数大于 1，只减少引用计数
	if (pinode->reference_count > 1)
	{
		pinode->reference_count--;
		return;
	}

	// 2. 如果引用计数不为 0，将内存 i-node 写回磁盘
	if (pinode->reference_count != 0)
	{
		addr = DISK_INODE_START_POINTOR + pinode->disk_inode_number * DISK_INODE_SIZE;

		// 构建磁盘 i-node
		DiskINode d_inode;
		d_inode.mode = pinode->mode;
		d_inode.owner_uid = pinode->owner_uid;
		d_inode.owner_gid = pinode->owner_gid;
		d_inode.file_size = pinode->file_size;
		memcpy(d_inode.block_addresses, pinode->block_addresses, sizeof(d_inode.block_addresses));

		memcpy(disk + addr, &d_inode, sizeof(DiskINode));
	}
	else // 3. 如果引用计数为 0，释放磁盘资源
	{
		uint32_t block_num = (pinode->file_size + BLOCK_SIZE - 1) / BLOCK_SIZE; // 向上取整
		for (uint32_t i = 0; i < block_num; i++)
			bfree(pinode->block_addresses[i]);

		ifree(pinode->disk_inode_number);
	}

	// 4. 从哈希链表中删除内存 i-node
	int inodeid = pinode->disk_inode_number % NHINO;

	if (hinode[inodeid].prev_inode == pinode) // 如果是链表头
	{
		hinode[inodeid].prev_inode = pinode->next;
	}
	else // 链表中的其他位置
	{
		if (pinode->next)
			pinode->next->prev = pinode->prev;
		if (pinode->prev)
			pinode->prev->next = pinode->next;
	}

	// 5. 释放内存 i-node
	free(pinode);
}
