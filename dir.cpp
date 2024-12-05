#include <stdio.h>
#include <string.h>
#include "filesys.h"

void _dir()
{
	unsigned int di_mode;
	int i, j, k; // xiao
	struct INode *temp_inode;

	printf("\n CURRENT DIRECTORY :%s\n", Dir.direct[0].d_name);
	printf("当前共有%d个文件/目录\n", Dir.size);
	for (i = 0; i < DIRNUM; i++)
	{
		if (Dir.direct[i].d_ino != DIEMPTY)
		{
			printf("%-14s", Dir.direct[i].d_name);
			temp_inode = iget(Dir.direct[i].d_ino);
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
	struct INode *inode;
	struct Direct buf[BLOCKSIZ / (DIRSIZ + 4)];
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
	dirpos = iname(dirname);				 // 取得在addr中的空闲项位置,并将目录名写到此项里
	inode = ialloc();						 // 分配i节点
	Dir.direct[dirpos].d_ino = inode->i_ino; // 设置该目录的磁盘i节点号
	Dir.size++;								 // 目录数++

	strcpy(buf[0].d_name, ".."); // 子目录的上一层目录 当前目录
	buf[0].d_ino = cur_path_inode->i_ino;
	strcpy(buf[1].d_name, ".");
	buf[1].d_ino = inode->i_ino; // 子目录的本目录 子目录

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
	inode = iget(Dir.direct[dirid].d_ino);
	if (!(inode->di_mode & DIDIR))
	{
		printf("不是一个目录！\n");
		return;
	}
	for (unsigned int i = 0; i < Dir.size; i++)
	{
		if (Dir.direct[i].d_ino == 0)
		{
			for (j = DIRNUM - 1; j >= 0 && Dir.direct[j].d_ino == 0; j--)
				;
			memcpy(&Dir.direct[i], &Dir.direct[j], DIRSIZ + 4); // xiao
			Dir.direct[j].d_ino = 0;
		}
	}
	j = cur_path_inode->di_size % BLOCKSIZ ? 1 : 0;
	for (unsigned short i = 0; i < cur_path_inode->di_size / BLOCKSIZ + j; i++)
	{
		bfree(cur_path_inode->di_addr[i]);
	}
	for (unsigned int i = 0; i < Dir.size; i += BLOCKSIZ / (DIRSIZ + 4))
	{
		block = balloc();
		cur_path_inode->di_addr[i] = block;
		memcpy(disk + DATASTART + block * BLOCKSIZ, &Dir.direct[i], BLOCKSIZ);
	}
	cur_path_inode->di_size = Dir.size * (DIRSIZ + 4);
	iput(cur_path_inode);
	cur_path_inode = inode;

	j = 0;
	for (unsigned short i = 0; i < inode->di_size / BLOCKSIZ + 1; i++)
	{
		memcpy(&Dir.direct[j], disk + DATASTART + inode->di_addr[i] * BLOCKSIZ, BLOCKSIZ);
		j += BLOCKSIZ / (DIRSIZ + 4);
	}
	Dir.size = cur_path_inode->di_size / (DIRSIZ + 4);
	for (unsigned int i = Dir.size; i < DIRNUM; i++)
	{
		Dir.direct[i].d_ino = 0;
	}

	// end by xiao

	return;
}
