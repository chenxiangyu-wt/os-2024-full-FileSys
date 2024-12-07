#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filesys.hpp"

/* 函数名：	iget 							*/
/* 作用：	为磁盘i结点分配对应的内存i结点	*/
/* 参数:	待分配的磁盘i结点号				*/
/* 返回值：	指向对应的内存i结点的指针		*/
MemoryINode *iget(unsigned int dinodeid)
{
	int existed = 0, inodeid;
	long addr;
	struct MemoryINode *temp, *newinode;

	inodeid = dinodeid % NHINO;				// 计算内存结点应该在第几个哈希队列里
	if (hinode[inodeid].prev_inode == NULL) // 若该哈希队列为空，内存结点一定未被创建
		existed = 0;
	else // 若不为空，从该哈希队列头开始查找
	{
		temp = hinode[inodeid].prev_inode;
		while (temp)
		{
			if (temp->status_flag == dinodeid) // 若找到
			{
				existed = 1;
				temp->reference_count++;
				return temp; // 返回该内存结点指针
			}
			else
				temp = temp->prev;
		}
	}

	/* 若没有找到 */
	/* 1. 计算该磁盘i结点在文件卷中的位置 */
	addr = DISK_INODE_START_POINTOR + dinodeid * DISK_INODE_SIZE;

	/* 2. 分配一个内存i结点 */
	newinode = (struct MemoryINode *)malloc(sizeof(MemoryINode));

	/* 3. 用磁盘i结点初始化内存i结点 */
	memcpy(&(newinode->reference_count), disk + addr, DISK_INODE_SIZE);

	/* 4. 将内存i结点链入相应的哈希队列里*/
	newinode->prev = hinode[inodeid].prev_inode;
	hinode[inodeid].prev_inode = newinode;
	newinode->next = newinode;
	if (newinode->prev)
		newinode->prev->next = newinode;

	/*5. 初始化内存i结点的其他数据项 */
	newinode->reference_count = 1;
	newinode->status_flag = 0; /* 表示未更新 */
	newinode->status_flag = dinodeid;

	return newinode;
}

/* 函数名：	iput							*/
/* 作用：	回收内存i结点					*/
/* 参数:	指向待回收的内存i结点指针		*/
/* 返回值：	无								*/
void iput(struct MemoryINode *pinode)
{
	long addr;
	unsigned int block_num;

	if (pinode->reference_count > 1) // 若引用计数>1
	{
		pinode->reference_count--;

		return;
	}
	else
	{
		if (pinode->reference_count != 0) // 若联结计数不为0
		{
			/* 把内存i结点的内容写回磁盘i结点 */
			addr = DISK_INODE_START_POINTOR + pinode->status_flag * DISK_INODE_SIZE;
			memcpy(disk + addr, &pinode->reference_count, DISK_INODE_SIZE);
		}
		else
		{
			/* 删除磁盘i结点和文件对应的物理块 */
			block_num = pinode->file_size / BLOCK_SIZE;
			for (unsigned int i = 0; i < block_num; i++)
				bfree(pinode->block_addresses[i]);
			ifree(pinode->status_flag);
		}

		/* 释放内存i结点 */
		{
			int inodeid;
			inodeid = (pinode->status_flag) % NHINO; // 找到所在的哈希队列

			/* 从该哈希队列里删除 */
			if (hinode[inodeid].prev_inode == pinode)
			{
				hinode[inodeid].prev_inode = pinode->prev;
				if (pinode->prev)
					pinode->prev->next = pinode->prev;
			}
			else
			{
				pinode->next->prev = pinode->prev;
				if (pinode->prev)
					pinode->prev->next = pinode->next;
			}
		}
		free(pinode);
	}

	return;
}
