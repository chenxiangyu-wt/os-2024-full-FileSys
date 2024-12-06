#include <stdio.h>
#include "filesys.h"

void removeFile(const char *filename)
{
	int dirid;
	struct INode *inode;

	dirid = namei(filename);
	if (dirid == -1)
	{
		printf("文件不存在，请检查!\n");
		return;
	}
	inode = iget(dir.entries[dirid].inode_number);
	if (!(inode->di_mode & DIFILE))
	{
		printf("对象不是文件，请检查！\n");
		iput(inode);
		return;
	}
	dir.entries[dirid].inode_number = DIEMPTY;
	dir.entry_count--;
	inode->di_number--;
	iput(inode);
	return;
}
