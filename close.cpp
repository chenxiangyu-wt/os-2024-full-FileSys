#include <stdio.h>
#include "file_sys.hpp"
#include "globals.hpp"

/******************************************************
函数：close
功能：关闭打开的文件，更新用户和系统的文件表，
	  并在引用计数为0时释放 inode。
参数：
- user_id: 用户ID
- cfd: 用户文件描述符
*******************************************************/
void close(uint32_t user_id, uint16_t cfd)
{
	// 1. 验证文件描述符是否合法
	if (cfd >= NOFILE || user[user_id].open_files[cfd] == SYSTEM_MAX_OPEN_FILE_NUM + 1)
	{
		printf("Error: Invalid file descriptor or file already closed.\n");
		return;
	}

	// 2. 获取系统文件表索引和对应的 inode
	int sys_file_index = user[user_id].open_files[cfd];
	MemoryINode *inode = system_opened_file[sys_file_index].inode;

	if (inode == NULL)
	{
		printf("Error: No inode associated with this file descriptor.\n");
		return;
	}

	// 3. 更新系统文件表的引用计数
	system_opened_file[sys_file_index].reference_count--;

	// 4. 如果引用计数为 0，释放 inode（iput 负责释放和写回）
	if (system_opened_file[sys_file_index].reference_count == 0)
	{
		// printf("Releasing inode %d and writing back to disk...\n", inode->disk_inode_number);
		iput(inode);									 // iput 函数会检查引用计数并执行释放逻辑
		system_opened_file[sys_file_index].inode = NULL; // 清空 inode 指针
	}

	// 5. 更新用户文件表，标记文件描述符为未使用
	user[user_id].open_files[cfd] = SYSTEM_MAX_OPEN_FILE_NUM + 1;

	// printf("File descriptor %d closed successfully.\n", cfd);
	return;
}
