#include <stdio.h>
#include <string.h>
#include "file_sys.hpp"
#include "globals.hpp"

/*
namei函数：参数：要查找的文件名或者目录名。返回文件对应的内存目录项标号i；如果返回-1表示没有该文件。
功能：查找文件在当前目录下对应的内存目录项的号；
*/
int namei(const char *filename, uint16_t type)
{
	for (int i = 0; i < dir.entry_count; ++i)
	{
		if (strcmp(dir.entries[i].name, filename) == 0 && (dir.entries[i].type & type))
		{
			return i; // 返回匹配项的索引
		}
	}
	return -1; // 未找到
}

/*
iname函数：功能：如果还有空目录项，为当前文件分配目录项；
		   参数：将要分配的文件名。
		   返回：0没有空闲目录项；i分配的目录项号。
*/
uint16_t iname(const char *name)
{
	int i, notfound = 1;

	for (i = 0; ((i < ENTRY_NUM) && (notfound)); i++)
	{
		if (dir.entries[i].inode_number == 0) // 该目录项未分配。
		{
			notfound = 0;
			break;
		}
	}

	if (notfound)
	{
		printf("\nThe current directory is full!!!\n");
		return 0;
	}
	strcpy(dir.entries[i].name, name);
	return i;
}
