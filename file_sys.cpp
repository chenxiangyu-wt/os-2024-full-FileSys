#include <cstring>
#include <iostream>
#include "globals.hpp"
#include "helper.hpp"

void format()
{
    MemoryINode *inode;
    uint32_t PASSWORD_FILE_LIMITE = 32;
    DirectoryEntry dir_buf[BLOCK_SIZE / sizeof(DirectoryEntry)];
    UserPassword passwd[PASSWORD_FILE_LIMITE];

    // 1. 初始化磁盘
    memset(disk, 0, DISK_SIZE);

    // 2. 初始化密码文件内容
    memset(passwd, 0, sizeof(passwd));
    passwd[0] = {2116, 3, "dddd"};
    passwd[1] = {2117, 3, "bbbb"};
    passwd[2] = {2118, 4, "abcd"};
    passwd[3] = {2119, 4, "cccc"};
    passwd[4] = {2120, 5, "eeee"};

    for (uint32_t i = 5; i < PASSWORD_FILE_LIMITE; i++)
    {
        passwd[i].user_id = 0;
        passwd[i].group_id = 0;
        memset(passwd[i].password, ' ', PWDSIZ);
    }

    // 3. 初始化超级块 (FileSystem)
    fileSystem.inode_block_count = DISK_INODE_AREA_SIZE;
    fileSystem.data_block_count = DATA_BLOCK_AREA_SIZE;

    fileSystem.free_inode_count = (DISK_INODE_AREA_SIZE * BLOCK_SIZE) / DISK_INODE_SIZE - 4;
    fileSystem.free_block_count = DATA_BLOCK_AREA_SIZE - 3;

    for (int i = 0; i < NICINOD; i++)
        fileSystem.free_inodes[i] = i + 4;
    fileSystem.free_inode_pointer = 0;
    fileSystem.last_allocated_inode = NICINOD + 4;

    int block_index = DATA_BLOCK_AREA_SIZE - 1;
    for (int i = 0; i < NICFREE; i++)
        fileSystem.free_blocks[i] = block_index--;
    fileSystem.free_block_pointer = 0;

    // 4. 创建根目录 (inode 1)
    inode = iget(1);
    inode->mode = DIDIR | 0755;
    inode->file_size = 3 * sizeof(DirectoryEntry);
    inode->reference_count = 1;
    inode->block_addresses[0] = balloc();

    memset(dir_buf, 0, sizeof(dir_buf));
    strcpy(dir_buf[0].name, "..");
    dir_buf[0].inode_number = 1;
    dir_buf[0].type = DENTRY_DIR;

    strcpy(dir_buf[1].name, ".");
    dir_buf[1].inode_number = 1;
    dir_buf[1].type = DENTRY_DIR;

    strcpy(dir_buf[2].name, "etc");
    dir_buf[2].inode_number = 2;
    dir_buf[2].type = DENTRY_DIR;

    memcpy(disk + DATA_START_POINTOR + inode->block_addresses[0] * BLOCK_SIZE, dir_buf, BLOCK_SIZE);
    iput(inode);

    // 5. 创建 `etc` 目录 (inode 2)
    inode = iget(2);
    inode->mode = DIDIR | 0755;
    inode->file_size = 3 * sizeof(DirectoryEntry);
    inode->reference_count = 1;
    inode->block_addresses[0] = balloc();

    memset(dir_buf, 0, sizeof(dir_buf));
    strcpy(dir_buf[0].name, "..");
    dir_buf[0].inode_number = 1;
    dir_buf[0].type = DENTRY_DIR;

    strcpy(dir_buf[1].name, ".");
    dir_buf[1].inode_number = 2;
    dir_buf[1].type = DENTRY_DIR;

    strcpy(dir_buf[2].name, "password");
    dir_buf[2].inode_number = 3;
    dir_buf[2].type = DENTRY_FILE;

    memcpy(disk + DATA_START_POINTOR + inode->block_addresses[0] * BLOCK_SIZE, dir_buf, BLOCK_SIZE);
    iput(inode);

    // 6. 创建 `password` 文件 (inode 3)
    inode = iget(3);
    inode->mode = DIFILE | 0644;
    inode->file_size = BLOCK_SIZE;
    inode->reference_count = 1;
    inode->block_addresses[0] = balloc();

    memcpy(disk + DATA_START_POINTOR + inode->block_addresses[0] * BLOCK_SIZE, passwd, sizeof(passwd));
    iput(inode);

    // 7. 将超级块写入磁盘
    memcpy(disk + BLOCK_SIZE, &fileSystem, sizeof(FileSystem));

    std::cout << "Format completed. File system initialized." << std::endl;
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
        system_opened_file[i].inode = nullptr;     // 清空 inode 指针
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
    cwd = iget(1);
    if (!cwd)
    {
        std::cerr << "Error: Failed to load root directory inode." << std::endl;
        return;
    }

    std::cout << "Root inode file size: " << cwd->file_size << " bytes" << std::endl;

    // 计算根目录的目录项数量
    dir.entry_count = cwd->file_size / sizeof(DirectoryEntry);
    std::cout << "Number of directory entries: " << dir.entry_count << std::endl;

    // 初始化内存中的目录结构（清空）
    for (uint32_t i = 0; i < ENTRY_NUM; i++)
    {
        strcpy(dir.entries[i].name, "             "); // 清空名称
        dir.entries[i].inode_number = 0;              // 清空 i-node 编号
    }

    // 遍历根目录的所有数据块，将目录项数据加载到内存
    uint32_t entries_per_block = BLOCK_SIZE / sizeof(DirectoryEntry);
    uint32_t blocks_needed = (dir.entry_count + entries_per_block - 1) / entries_per_block; // 向上取整，计算所需数据块数

    std::cout << "Entries per block: " << entries_per_block << ", Blocks needed: " << blocks_needed << std::endl;

    // 遍历每个数据块，加载目录项
    for (uint32_t i = 0; i < blocks_needed; i++)
    {
        uint32_t offset = DATA_START_POINTOR + BLOCK_SIZE * cwd->block_addresses[i];
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
    // 加载密码文件到内存
    std::cout << "Loading password file into memory..." << std::endl;
    MemoryINode *password_inode = iget(3);
    if (!password_inode)
    {
        std::cerr << "Error: Failed to load password file inode." << std::endl;
        return;
    }
    // 读取密码文件数据块
    uint32_t password_block = password_inode->block_addresses[0];
    memcpy(pwd, disk + DATA_START_POINTOR + password_block * BLOCK_SIZE, sizeof(pwd));

    std::cout << "Directory entries loaded successfully." << std::endl;
    std::cout << "File system installation complete." << std::endl;
}

void halt()
{
    int i, j;

    /*1. write back the current dir */
    chdir("..");
    iput(cwd);

    /*2. free the u_ofile and sys_ofile and inode*/
    for (i = 0; i < USERNUM; i++)
    {
        if (user[i].user_id != 0)
        {
            for (j = 0; j < NOFILE; j++)
            {
                if (user[i].open_files[j] != SYSTEM_MAX_OPEN_FILE_NUM + 1)
                {
                    closeFile(i, j);
                    user[i].open_files[j] = SYSTEM_MAX_OPEN_FILE_NUM + 1;
                } // if
            } // for
        } // if
    } // for

    /*3. write back the filesys to the disk*/
    memcpy(disk + BLOCK_SIZE, &fileSystem, sizeof(FileSystem));

    /*4. close the file system column*/

    /*5. say GOOD BYE to all the user*/
    printf("\nGood Bye. See You Next Time. Please turn off the switch\n");
    return;
}

static uint32_t block_buf[BLOCK_SIZE];
/**********************************************************
函数：balloc
功能：维护超级块中的空闲数据栈，分配空闲数据块，并返回其块号
***********************************************************/
uint32_t balloc()
{
    uint32_t free_block;
    int i;

    // 如果没有空闲盘块
    if (fileSystem.free_block_count == 0)
    {
        printf("\nDisk Full!!!\n");
        return DISKFULL;
    }
    free_block = fileSystem.free_blocks[fileSystem.free_block_pointer]; // 取堆栈中的盘块号
    if (fileSystem.free_block_pointer == NICFREE - 1)
    { // 如果堆栈只剩一个块
        memcpy(block_buf, disk + DATA_START_POINTOR + (free_block)*BLOCK_SIZE, BLOCK_SIZE);
        // 从中读取下一组块号
        for (i = 0; i < NICFREE; i++)
            fileSystem.free_blocks[i] = block_buf[i];
        fileSystem.free_block_pointer = 0; // 设置堆栈指针
    }
    else
    {                                    // 如果堆栈中大于一个盘块
        fileSystem.free_block_pointer++; // 修改堆栈指针
    }
    fileSystem.free_block_count--; // 修改总块数
    fileSystem.superblock_modified_flag = SUPDATE;
    return free_block;
}

void bfree(uint32_t block_num)
{
    int i;

    if (fileSystem.free_block_pointer == 0)
    { // 如果堆栈已满
        /*将当前堆栈内块号写入当前块号*/
        for (i = 0; i < NICFREE; i++)
            block_buf[i] = fileSystem.free_blocks[NICFREE - 1 - i];
        memcpy(disk + DATA_START_POINTOR + block_num * BLOCK_SIZE, block_buf, BLOCK_SIZE);
        fileSystem.free_block_pointer = NICFREE; // 清空堆栈
    }
    /*修改堆栈指针，并将当前块号压入堆栈*/
    fileSystem.free_block_pointer--;
    fileSystem.free_block_count++;
    fileSystem.free_blocks[fileSystem.free_block_pointer] = block_num;
    fileSystem.superblock_modified_flag = SUPDATE;
    return;
}
