#include "filesys.h"

/**********************************************************************************
函数：access
参数：用户ID号；内存节点；要判断的权限
功能：通过用户权限判断用户是否能对该文件进行相应的操作；权限够返回1，权限不够返回0。
***********************************************************************************/
unsigned int access(unsigned int user_id, struct MemoryINode *inode, unsigned short mode)
{
	switch (mode)
	{
	case READ:
		if (inode->mode & ODIREAD) // 文件是所有用户可读行通过；
			return 1;
		if ((inode->mode & GDIREAD) && (user[user_id].group_id == inode->owner_gid)) // 文件本组可读，用户组和文件组一致通过；
			return 1;
		if ((inode->mode & UDIREAD) && (user[user_id].user_id == inode->owner_uid)) // 文件指定用户可读，是该用户通过；
			return 1;
		return 0; // 其他不通过；
	case WRITE:
		if (inode->mode & ODIWRITE)
			return 1;
		if ((inode->mode & GDIWRITE) && (user[user_id].group_id == inode->owner_gid))
			return 1;
		if ((inode->mode & UDIWRITE) && (user[user_id].user_id == inode->owner_uid))
			return 1;
		return 0;
	case EXICUTE:
		if (inode->mode & ODIEXICUTE)
			return 1;
		if ((inode->mode & GDIEXICUTE) && (user[user_id].group_id == inode->owner_gid))
			return 1;
		if ((inode->mode & UDIEXICUTE) && (user[user_id].user_id == inode->owner_uid))
			return 1;
		return 0;
	} // swith
	return 0;
}
