#include <stdio.h>
#include <string.h>
#include "filesys.h"

void install()
{

	/* 1. read the filsys from the superblock*/ // xiao
	memcpy(&FileSystem, disk + BLOCKSIZ, sizeof(struct FileSystem));

	/* 2. initialize the inode hash chain*/
	for (unsigned int i = 0; i < NHINO; i++)
	{
		hinode[i].i_forw = NULL;
	}

	/*3. initialize the sys_ofile*/
	for (unsigned int i = 0; i < SYSOPENFILE; i++)
	{
		sys_ofile[i].f_count = 0;
		sys_ofile[i].f_inode = NULL;
	}

	/*4. initialize the user*/
	for (unsigned int i = 0; i < USERNUM; i++)
	{
		user[i].u_uid = 0;
		user[i].u_gid = 0;
		for (unsigned int j = 0; j < NOFILE; j++)
			user[i].u_ofile[j] = SYSOPENFILE + 1;
	}

	/*5. read the main directory to initialize the dir*/
	cur_path_inode = iget(1);
	Dir.size = cur_path_inode->di_size / (DIRSIZ + 4); // xiao 2-->4

	for (unsigned int i = 0; i < DIRNUM; i++)
	{
		strcpy(Dir.direct[i].d_name, "             ");
		Dir.direct[i].d_ino = 0;
	}
	unsigned int i;
	for (i = 0; i < Dir.size / (BLOCKSIZ / (DIRSIZ + 4)); i++)
	{
		memcpy(&Dir.direct[(BLOCKSIZ / (DIRSIZ + 4)) * i],
			   disk + DATASTART + BLOCKSIZ * cur_path_inode->di_addr[i], DINODESIZ);
	}

	memcpy(&Dir.direct[(BLOCKSIZ) / (DIRSIZ + 4) * i],
		   disk + DATASTART + BLOCKSIZ * cur_path_inode->di_addr[i], DINODESIZ);
	return;
}
