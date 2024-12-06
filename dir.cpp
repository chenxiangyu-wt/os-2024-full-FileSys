#include <stdio.h>
#include <string.h>
#include "filesys.h"

void _dir()
{
	unsigned int di_mode;
	int i, j, k; // xiao
	struct INode *temp_inode;

	printf("\n CURRENT DIRECTORY :%s\n", dir.entries[0].name);
	printf("当前共有%d个文件/目录\n", dir.entry_count);
	for (i = 0; i < DIRNUM; i++)
	{
		if (dir.entries[i].inode_number != DIEMPTY)
		{
			printf("%-14s", dir.entries[i].name);
			temp_inode = iget(dir.entries[i].inode_number);
			di_mode = temp_inode->di_mode & 00777;
			for (j = 0; j < 9; j++)
			{
				if (di_mode % 2)
				{
					printf("x");
				}
				else
				{
					printf("-");
				}
				di_mode = di_mode / 2;
			}
			printf("\ti_ino->%d\t", temp_inode->i_ino);
			if (temp_inode->di_mode & DIFILE)
			{
				printf(" %d ", temp_inode->di_size);
				printf("block chain:");
				j = (temp_inode->di_size % BLOCKSIZ) ? 1 : 0;
				for (k = 0; k < temp_inode->di_size / BLOCKSIZ + j; k++)
					printf("%4d", temp_inode->di_addr[k]);
				printf("\n");
			}
			else
			{
				printf("<dir>\n");
			} // else
			iput(temp_inode);
		} // if (dir.direct[i].d_ino != DIEMPTY)
	} // for
	return;
}
void mkdir(const char *dirname)
{
	int dirid, dirpos;
	INode *inode;
	DirectoryEntry buf[BLOCKSIZ / (DIRSIZ + 4)];
	unsigned int block;

	dirid = namei(dirname);
	if (dirid != -1)
	{
		inode = iget(dirid);
		if (inode->di_mode & DIDIR)
			printf("目录%s已存在！\n", dirname); // xiao
		else
			printf("%s是一个文件！\n", dirname);
		iput(inode);
		return;
	}
	dirpos = iname(dirname);						 // 取得在addr中的空闲项位置,并将目录名写到此项里
	inode = ialloc();								 // 分配i节点
	dir.entries[dirpos].inode_number = inode->i_ino; // 设置该目录的磁盘i节点号
	dir.entry_count++;								 // 目录数++

	strcpy(buf[0].name, ".."); // 子目录的上一层目录 当前目录
	buf[0].inode_number = cur_path_inode->i_ino;
	strcpy(buf[1].name, ".");
	buf[1].inode_number = inode->i_ino; // 子目录的本目录 子目录

	block = balloc();
	memcpy(disk + DATASTART + block * BLOCKSIZ, buf, BLOCKSIZ);

	inode->di_size = 2 * (DIRSIZ + 4);
	inode->di_number = 1;
	inode->di_mode = user[user_id].u_default_mode | DIDIR;
	inode->di_uid = user[user_id].u_uid;
	inode->di_gid = user[user_id].u_gid;
	inode->di_addr[0] = block;

	iput(inode);
	return;
}

void chdir(const char *dirname)
{
	int dirid;
	struct INode *inode;
	unsigned short block;
	int j, low = 0, high = 0;

	dirid = namei(dirname);
	if (dirid == -1)
	{
		printf("不存在目录%s！\n", dirname);
		return;
	}
	inode = iget(dir.entries[dirid].inode_number);
	if (!(inode->di_mode & DIDIR))
	{
		printf("不是一个目录！\n");
		return;
	}
	for (unsigned int i = 0; i < dir.entry_count; i++)
	{
		if (dir.entries[i].inode_number == 0)
		{
			for (j = DIRNUM - 1; j >= 0 && dir.entries[j].inode_number == 0; j--)
				;
			memcpy(&dir.entries[i], &dir.entries[j], DIRSIZ + 4); // xiao
			dir.entries[j].inode_number = 0;
		}
	}
	j = cur_path_inode->di_size % BLOCKSIZ ? 1 : 0;
	for (unsigned short i = 0; i < cur_path_inode->di_size / BLOCKSIZ + j; i++)
	{
		bfree(cur_path_inode->di_addr[i]);
	}
	for (unsigned int i = 0; i < dir.entry_count; i += BLOCKSIZ / (DIRSIZ + 4))
	{
		block = balloc();
		cur_path_inode->di_addr[i] = block;
		memcpy(disk + DATASTART + block * BLOCKSIZ, &dir.entries[i], BLOCKSIZ);
	}
	cur_path_inode->di_size = dir.entry_count * (DIRSIZ + 4);
	iput(cur_path_inode);
	cur_path_inode = inode;

	j = 0;
	for (unsigned short i = 0; i < inode->di_size / BLOCKSIZ + 1; i++)
	{
		memcpy(&dir.entries[j], disk + DATASTART + inode->di_addr[i] * BLOCKSIZ, BLOCKSIZ);
		j += BLOCKSIZ / (DIRSIZ + 4);
	}
	dir.entry_count = cur_path_inode->di_size / (DIRSIZ + 4);
	for (unsigned int i = dir.entry_count; i < DIRNUM; i++)
	{
		dir.entries[i].inode_number = 0;
	}

	// end by xiao

	return;
}
