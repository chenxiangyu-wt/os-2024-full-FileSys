#include <stdio.h>
#include <string.h>
#include "filesys.h"
#include "log.h"

int find_user_index(unsigned short uid, const char *passwd)
{
	for (int i = 0; i < PWDNUM; i++)
	{
		if (uid == pwd[i].user_id && strcmp(passwd, pwd[i].password) == 0)
		{
			return i; // 找到匹配的用户
		}
	}
	return -1; // 未找到用户
}
int find_free_user_slot()
{
	for (int j = 0; j < USERNUM; j++)
	{
		if (user[j].user_id == 0)
		{
			return j; // 找到空闲位置
		}
	}
	return -1; // 无空闲位置
}

int login(unsigned short uid, const char *passwd)
{
	int user_index = find_user_index(uid, passwd);
	if (user_index == -1)
	{
		printf("\nIncorrect password\n");
		return ERR_INCORRECT_PASSWORD;
	}

	int free_slot = find_free_user_slot();
	if (free_slot == -1)
	{
		printf("\nToo many users in the system, please wait\n");
		return ERR_TOO_MANY_USERS;
	}

	// 将用户信息写入空闲位置
	user[free_slot].user_id = uid;
	user[free_slot].group_id = pwd[user_index].group_id;
	user[free_slot].default_mode = DEFAULTMODE;

	return LOGIN_SUCCESS;
}
// int login(unsigned short uid, char *passwd)
// {
// 	int i, j;

// 	for (i = 0; i < PWDNUM; i++)
// 	{
// 		if ((uid == pwd[i].p_uid) && (!strcmp(passwd, pwd[i].password)))
// 		{
// 			for (j = 0; j < USERNUM; j++)
// 			{
// 				if (user[j].u_uid == 0)
// 					break;
// 			}

// 			if (j == USERNUM)
// 			{
// 				printf("\ntoo much user in the systemm, waited to login\n");
// 				return -1;
// 			}
// 			else
// 			{
// 				user[j].u_uid = uid;
// 				user[j].u_gid = pwd[i].p_gid;
// 				user[j].u_default_mode = DEFAULTMODE;
// 			}
// 			break;
// 		}
// 	}
// 	if (i == PWDNUM)
// 	{
// 		printf("\nincorrect password\n");
// 		return -1;
// 	}
// 	else
// 	{
// 		return j;
// 	}
// }
int logout(unsigned short uid)
{
	int i, j, sys_no;
	struct INode *inode;

	for (i = 0; i < USERNUM; i++)
	{
		if (uid == user[i].user_id)
			break;
	}

	if (i == USERNUM)
	{
		printf("\nno such a file\n");
		return 0;
	}

	for (j = 0; j < NOFILE; j++)
	{
		if (user[i].open_files[j] != SYSOPENFILE + 1)
		{
			/* iput the inode free the sys_ofile and clear the user_ofile */
			sys_no = user[i].open_files[j];
			inode = sys_ofile[sys_no].inode;
			iput(inode);
			sys_ofile[sys_no].reference_count--;
			user[i].open_files[j] = SYSOPENFILE + 1;
		}
	}
	return 1;
}
