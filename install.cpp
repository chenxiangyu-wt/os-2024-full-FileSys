#include <iostream>
#include <cstring>
#include <iomanip>
#include "file_sys.hpp"
#include "globals.hpp"

void printFileSystem(const FileSystem &fs)
{
	std::cout << "\n===== File System Superblock =====\n";
	std::cout << std::left << std::setw(30) << "Field" << "Value\n";
	std::cout << "---------------------------------------------\n";

	std::cout << std::left << std::setw(30) << "Inode Block Count:" << fs.inode_block_count << "\n";
	std::cout << std::left << std::setw(30) << "Data Block Count:" << fs.data_block_count << "\n";
	std::cout << std::left << std::setw(30) << "Free Block Count:" << fs.free_block_count << "\n";
	std::cout << std::left << std::setw(30) << "Free Block Pointer:" << fs.free_block_pointer << "\n";

	// 打印 Free Blocks 堆栈
	std::cout << std::left << std::setw(30) << "Free Blocks Stack:" << "\n";
	for (int i = 0; i < NICFREE; i++)
	{
		std::cout << std::setw(5) << fs.free_blocks[i];
		if ((i + 1) % 10 == 0)
			std::cout << "\n";
	}
	std::cout << "\n";

	std::cout << std::left << std::setw(30) << "Free Inode Count:" << fs.free_inode_count << "\n";
	std::cout << std::left << std::setw(30) << "Free Inode Pointer:" << fs.free_inode_pointer << "\n";

	// 打印 Free Inodes 数组
	std::cout << std::left << std::setw(30) << "Free Inodes Array:" << "\n";
	for (int i = 0; i < NICINOD; i++)
	{
		std::cout << std::setw(5) << fs.free_inodes[i];
		if ((i + 1) % 10 == 0)
			std::cout << "\n";
	}
	std::cout << "\n";

	std::cout << std::left << std::setw(30) << "Last Allocated Inode:" << fs.last_allocated_inode << "\n";
	std::cout << std::left << std::setw(30) << "Superblock Modified Flag:" << static_cast<int>(fs.superblock_modified_flag) << "\n";
	std::cout << "===============================================\n";
}

// 安装文件系统：将磁盘中的超级块、根目录和其他关键数据加载到内存
void install()
{
	std::cout << "Installing the file system..." << std::endl;

	// 1. 读取超级块到内存
	std::cout << "Step 1: Loading Superblock into memory..." << std::endl;
	std::cout << "Size of FileSystem: " << sizeof(FileSystem) << " bytes" << std::endl;
	memcpy(&fileSystem, disk + BLOCK_SIZE, sizeof(FileSystem));
	printFileSystem(fileSystem);

	// 2. 初始化 i-node 哈希链表
	std::cout << "Step 2: Initializing inode hash chain..." << std::endl;
	for (uint32_t i = 0; i < NHINO; i++)
	{
		hinode[i].prev_inode = nullptr; // 清空哈希链表头指针
	}

	// 3. 初始化系统打开文件表
	std::cout << "Step 3: Initializing system open file table..." << std::endl;
	for (uint32_t i = 0; i < SYSTEM_MAX_OPEN_FILE_NUM; i++)
	{
		system_opened_file[i].reference_count = 0; // 设置引用计数为0
		system_opened_file[i].inode = nullptr;	   // 清空 inode 指针
	}

	// 4. 初始化用户上下文
	std::cout << "Step 4: Initializing user context..." << std::endl;
	for (uint32_t i = 0; i < USERNUM; i++)
	{
		user[i].user_id = 0;  // 设置用户ID为0（未登录）
		user[i].group_id = 0; // 设置用户组ID为0
		for (uint32_t j = 0; j < NOFILE; j++)
		{
			user[i].open_files[j] = SYSTEM_MAX_OPEN_FILE_NUM + 1; // 所有文件描述符初始化为未使用状态
		}
	}

	// 5. 加载根目录的 i-node 并初始化内存中的目录结构
	std::cout << "Step 5: Loading root directory inode and initializing directory..." << std::endl;

	// 加载根目录 i-node，编号为1
	cur_path_inode = iget(1);
	if (!cur_path_inode)
	{
		std::cerr << "Error: Failed to load root directory inode." << std::endl;
		return;
	}

	std::cout << "Root inode file size: " << cur_path_inode->file_size << " bytes" << std::endl;

	// 计算根目录的目录项数量
	dir.entry_count = cur_path_inode->file_size / sizeof(DirectoryEntry);
	std::cout << "Number of directory entries: " << dir.entry_count << std::endl;

	// 初始化内存中的目录结构（清空）
	for (uint32_t i = 0; i < ENTRY_NUM; i++)
	{
		strcpy(dir.entries[i].name, "             "); // 清空名称
		dir.entries[i].inode_number = 0;			  // 清空 i-node 编号
	}

	// 遍历根目录的所有数据块，将目录项数据加载到内存
	uint32_t entries_per_block = BLOCK_SIZE / sizeof(DirectoryEntry);
	uint32_t blocks_needed = (dir.entry_count + entries_per_block - 1) / entries_per_block; // 向上取整，计算所需数据块数

	std::cout << "Entries per block: " << entries_per_block << ", Blocks needed: " << blocks_needed << std::endl;

	// 遍历每个数据块，加载目录项
	for (uint32_t i = 0; i < blocks_needed; i++)
	{
		uint32_t offset = DATA_START_POINTOR + BLOCK_SIZE * cur_path_inode->block_addresses[i];
		std::cout << "Loading directory block " << i << " at offset: " << offset << std::endl;

		// 计算当前块需要拷贝的目录项数
		uint32_t entries_to_copy = entries_per_block;
		if (i == blocks_needed - 1) // 最后一个数据块
		{
			entries_to_copy = dir.entry_count % entries_per_block;
			if (entries_to_copy == 0) // 刚好整除时，拷贝完整块
				entries_to_copy = entries_per_block;
		}

		// 拷贝目录项数据
		memcpy(&dir.entries[entries_per_block * i],
			   disk + offset,
			   entries_to_copy * sizeof(DirectoryEntry));
	}

	std::cout << "Directory entries loaded successfully." << std::endl;
	std::cout << "File system installation complete." << std::endl;
}