#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include "dEntry.hpp"
#include "file_sys.hpp"
#include "globals.hpp"

short openFile(int user_id, const char *filename, char openmode)
{
    int disk_inode_id;
    MemoryINode *inode;
    uint32_t sys_file_index, user_fd_index;

    // 1. 查找目标文件的 inode
    disk_inode_id = namei(filename, DENTRY_FILE);
    if (disk_inode_id == 0)
    {
        printf("\nFile does not exist!!!\n");
        return -1;
    }

    inode = iget(dir.entries[disk_inode_id].inode_number);
    if (!(inode->mode & DIFILE))
    {
        printf("%s is not a file!!!\n", filename);
        iput(inode);
        return -1;
    }

    // 2. 检查用户权限
    if (!access(user_id, inode, openmode))
    {
        printf("\nAccess denied: No permission to open file.\n");
        iput(inode);
        return -1;
    }

    // 3. 查找系统打开文件表的空闲位置
    for (sys_file_index = 0; sys_file_index < SYSTEM_MAX_OPEN_FILE_NUM; sys_file_index++)
    {
        if (system_opened_file[sys_file_index].reference_count == 0)
        {
            break;
        }
    }
    if (sys_file_index == SYSTEM_MAX_OPEN_FILE_NUM)
    {
        printf("\nSystem open file table is full.\n");
        iput(inode);
        return -1;
    }

    // 4. 查找用户打开文件表的空闲位置
    for (user_fd_index = 0; user_fd_index < NOFILE; user_fd_index++)
    {
        if (user[user_id].open_files[user_fd_index] == SYSTEM_MAX_OPEN_FILE_NUM + 1)
        {
            break;
        }
    }
    if (user_fd_index == NOFILE)
    {
        printf("\nUser open file table is full.\n");
        iput(inode);
        return -1;
    }

    // 5. 更新系统和用户文件表
    system_opened_file[sys_file_index].inode = inode;
    system_opened_file[sys_file_index].flag = openmode;
    system_opened_file[sys_file_index].reference_count = 1;

    if (openmode & FAPPEND)
    {
        system_opened_file[sys_file_index].offset = inode->file_size;
    }
    else
    {
        system_opened_file[sys_file_index].offset = 0;
    }

    user[user_id].open_files[user_fd_index] = sys_file_index; // 关联系统文件表索引

    // printf("File '%s' opened successfully. User FD: %d\n", filename, user_fd_index);
    return user_fd_index; // 返回用户的文件描述符
}

void removeFile(const char *filename)
{
    int dirid;
    struct MemoryINode *inode;

    dirid = namei(filename, DENTRY_FILE);
    if (dirid == -1)
    {
        printf("文件不存在，请检查!\n");
        return;
    }
    inode = iget(dir.entries[dirid].inode_number);
    if (!(inode->mode & DIFILE))
    {
        printf("对象不是文件，请检查！\n");
        iput(inode);
        return;
    }
    dir.entries[dirid].inode_number = DIEMPTY;
    dir.entry_count--;
    inode->reference_count--;
    iput(inode);
    return;
}

/*********************************************************************
函数：creat
功能：创建文件，存在且可写则覆盖，否则申请i节点，并打开该文件，返回文件指针
**********************************************************************/
int creatFile(uint32_t user_id, const char *filename, uint16_t mode)
{
    struct MemoryINode *inode;
    int entry_index, empty_entry_index;

    // 1. 查找目录项，查看是否存在同名文件或目录
    entry_index = namei(filename, DENTRY_FILE | DENTRY_DIR);
    if (entry_index != -1)
    {
        inode = iget(dir.entries[entry_index].inode_number);
        if (inode == NULL)
        {
            printf("Error: Failed to load inode.\n");
            return -1;
        }

        if (inode->mode & DIDIR)
        {
            printf("存在同名目录，无法创建文件！\n");
        }
        else
        {
            printf("Error: 文件 %s 已存在，无法重复创建！\n", filename);
        }

        iput(inode);
        return -1;
    }

    // 2. 不存在同名文件，分配新 inode
    printf("创建新文件: %s\n", filename);

    inode = ialloc();
    if (!inode)
    {
        printf("Error: Failed to allocate inode.\n");
        return -1;
    }

    empty_entry_index = iname(filename);
    if (empty_entry_index == -1)
    {
        printf("Error: Directory is full, cannot create more files.\n");
        iput(inode);
        return -1;
    }

    strcpy(dir.entries[empty_entry_index].name, filename);
    dir.entries[empty_entry_index].inode_number = inode->disk_inode_number;
    dir.entries[empty_entry_index].type = DENTRY_FILE;
    dir.entry_count++;

    inode->mode = mode | DIFILE;
    inode->owner_uid = user[user_id].user_id;
    inode->owner_gid = user[user_id].group_id;
    inode->file_size = 0;
    inode->reference_count = 1;

    iput(inode);
    return openFile(user_id, filename, WRITE);
}

/******************************************************
函数：close
功能：关闭打开的文件，更新用户和系统的文件表，
      并在引用计数为0时释放 inode。
参数：
- user_id: 用户ID
- cfd: 用户文件描述符
*******************************************************/
void closeFile(uint32_t user_id, uint16_t cfd)
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
        iput(inode);                                     // iput 函数会检查引用计数并执行释放逻辑
        system_opened_file[sys_file_index].inode = NULL; // 清空 inode 指针
    }

    // 5. 更新用户文件表，标记文件描述符为未使用
    user[user_id].open_files[cfd] = SYSTEM_MAX_OPEN_FILE_NUM + 1;

    // printf("File descriptor %d closed successfully.\n", cfd);
    return;
}

uint32_t readFile(int fd, char *buf, uint32_t size)
{
    unsigned long off;
    uint32_t block, block_off, i, j;
    struct MemoryINode *inode;
    char *temp_buf;

    inode = system_opened_file[user[user_id].open_files[fd]].inode;
    if (!(system_opened_file[user[user_id].open_files[fd]].flag & FREAD))
    {
        printf("\nthe file is not opened for read\n");
        return 0;
    }
    temp_buf = buf;
    off = system_opened_file[user[user_id].open_files[fd]].offset;
    if ((off + size) > inode->file_size)
    {
        size = inode->file_size - off;
    }
    block_off = off % BLOCK_SIZE;
    block = off / BLOCK_SIZE;
    if (block_off + size < BLOCK_SIZE)
    {
        memcpy(buf, disk + DATA_START_POINTOR + inode->block_addresses[block] * BLOCK_SIZE + block_off, size);
        return size;
    }
    memcpy(temp_buf, disk + DATA_START_POINTOR + inode->block_addresses[block] * BLOCK_SIZE + block_off, BLOCK_SIZE - block_off);
    temp_buf += BLOCK_SIZE - block_off;
    j = (inode->file_size - off - block_off) / BLOCK_SIZE;
    for (i = 0; i < (size - (BLOCK_SIZE - block_off)) / BLOCK_SIZE; i++)
    {
        memcpy(temp_buf, disk + DATA_START_POINTOR + inode->block_addresses[j + i] * BLOCK_SIZE, BLOCK_SIZE);
        temp_buf += BLOCK_SIZE;
    }

    block_off = (size - (BLOCK_SIZE - block_off)) % BLOCK_SIZE;
    memcpy(temp_buf, disk + DATA_START_POINTOR + i * BLOCK_SIZE, block_off);
    system_opened_file[user[user_id].open_files[fd]].offset += size;
    return size;
}

uint32_t writeFile(int file_id, char *buf, uint32_t size)
{
    unsigned long offset;
    uint32_t logical_block, block_offset, i;
    MemoryINode *inode;
    char *temp_buf;
    uint32_t remaining_size = size; // 剩余要写入的数据大小

    // 获取文件对应的内存 i-node
    inode = system_opened_file[user[user_id].open_files[file_id]].inode;
    if (!(system_opened_file[user[user_id].open_files[file_id]].flag & FWRITE))
    {
        printf("Error: The file is not opened for write.\n");
        return 0;
    }

    // 当前写入位置
    offset = system_opened_file[user[user_id].open_files[file_id]].offset;

    // 计算逻辑块和块内偏移量
    logical_block = offset / BLOCK_SIZE; // 逻辑块号
    block_offset = offset % BLOCK_SIZE;  // 块内偏移量

    temp_buf = buf;

    // 循环写入数据
    while (remaining_size > 0)
    {
        // 检查是否超出 12 个直接块的限制
        if (logical_block >= 12)
        {
            std::cout << "Error: File size exceeds the limit of " << ADDRESS_POINTOR_NUM - 1 << " direct blocks.\n";
            break;
        }

        // 分配新的数据块（如果未分配）
        if (inode->block_addresses[logical_block] == 0)
        {
            inode->block_addresses[logical_block] = balloc();
            if (inode->block_addresses[logical_block] == 0) // 分配失败
            {
                printf("Error: Failed to allocate block.\n");
                break;
            }
        }

        // 计算当前块可以写入的大小
        uint32_t write_size = std::min(BLOCK_SIZE - block_offset, remaining_size);

        // 将数据写入磁盘对应的数据块
        memcpy(disk + DATA_START_POINTOR + inode->block_addresses[logical_block] * BLOCK_SIZE + block_offset,
               temp_buf, write_size);

        // 更新偏移量和剩余大小
        temp_buf += write_size;
        remaining_size -= write_size;
        offset += write_size;

        // 移动到下一个逻辑块
        logical_block++;
        block_offset = 0; // 下一个块从偏移量 0 开始
    }

    // 更新文件大小
    if (offset > inode->file_size)
    {
        inode->file_size = offset;
    }

    // 更新文件的偏移量
    system_opened_file[user[user_id].open_files[file_id]].offset = offset;

    // 返回实际写入的字节数
    return size - remaining_size;
}
