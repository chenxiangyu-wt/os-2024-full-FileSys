#include <stdio.h>
#include <string.h>
#include <stack>
#include <iostream>
#include <stdlib.h>
#include "file_sys.hpp"
#include "globals.hpp"
#include "PathResolver.hpp"

static struct DiskINode block_buf[BLOCK_SIZE / DISK_INODE_SIZE]; // 存放i节点的临时数组
/*****************************************************
函数：ialloc
功能：分配磁盘i节点，返回相应的内存i节点指针
******************************************************/
struct MemoryINode *ialloc()
{
    struct MemoryINode *temp_inode;
    uint32_t cur_di;
    int i, count, block_end_flag;

    // I界点分配时从低位到高位使用，并且分配的i节点也是由低到高
    if (fileSystem.free_inode_pointer == NICINOD)
    {
        i = 0;
        block_end_flag = 1;
        count = fileSystem.free_inode_pointer = fileSystem.free_inode_count > NICINOD ? 0 : (NICINOD - fileSystem.free_inode_count);
        cur_di = fileSystem.last_allocated_inode;
        while (count < NICINOD)
        { // 空闲i节点数组没有装满且磁盘中还有空闲i节点
            if (block_end_flag)
            {
                memcpy(block_buf, disk + DISK_INODE_START_POINTOR + cur_di * DISK_INODE_SIZE, BLOCK_SIZE); // 从i节点去中读一个盘块到临时数组
                block_end_flag = 0;
                i = 0;
            }
            while (block_buf[IUPDATE].mode != DIEMPTY)
            { // 临时数组为空，则读到空闲i节点数组中
                cur_di++;
                i++;
            }
            if (i == NICINOD)
            { // 空闲i节点数组已满
                block_end_flag = 1;
                continue;
            }
            fileSystem.free_inodes[count++] = cur_di;
        }
        fileSystem.last_allocated_inode = cur_di; // 重新设铭记i节点
    }
    /*分配空闲i节点*/
    temp_inode = iget(fileSystem.free_inodes[fileSystem.free_inode_pointer]);
    memcpy(disk + DISK_INODE_START_POINTOR + fileSystem.free_inodes[fileSystem.free_inode_pointer] * DISK_INODE_SIZE,
           &temp_inode->reference_count, sizeof(struct DiskINode));
    fileSystem.free_inode_pointer++;
    fileSystem.free_inode_count--;
    fileSystem.superblock_modified_flag = SUPDATE;
    return temp_inode;
}

void ifree(uint32_t dinodeid)
{
    fileSystem.free_inode_count--; // 空闲i节点数减一
    if (fileSystem.free_inode_pointer != 0)
    { // 空闲i节点数组未满
        fileSystem.free_inode_pointer--;
        fileSystem.free_inodes[fileSystem.free_inode_pointer] = dinodeid;
    }
    else
    {
        if (dinodeid < fileSystem.last_allocated_inode)
        {
            // 新释放i节点号小于铭记i节点号，则丢弃原铭记i节点，设新的铭记i节点为新释放的铭记i节点
            fileSystem.free_inodes[NICINOD] = dinodeid;
            fileSystem.last_allocated_inode = dinodeid;
        }
    }
    return;
}

/* 函数名：	iget 							*/
/* 作用：	为磁盘i结点分配对应的内存i结点	*/
/* 参数:	待分配的磁盘i结点号				*/
/* 返回值：	指向对应的内存i结点的指针		*/
MemoryINode *iget(uint32_t dinodeid)
{
    int inodeid = dinodeid % NHINO; // 计算哈希链表索引
    MemoryINode *temp = hinode[inodeid].prev_inode;

    // 1. 检查哈希链表中是否已存在该 i-node
    while (temp)
    {
        if (temp->inode_number == dinodeid) // 已存在
        {
            temp->reference_count++;
            return temp;
        }
        temp = temp->next;
    }

    // 2. 若没有找到，从磁盘加载 i-node
    long addr = DISK_INODE_START_POINTOR + dinodeid * DISK_INODE_SIZE;
    DiskINode disk_inode;

    memcpy(&disk_inode, disk + addr, sizeof(DiskINode)); // 读取磁盘 i-node

    // 3. 分配内存 i-node 并初始化
    MemoryINode *new_inode = (MemoryINode *)malloc(sizeof(MemoryINode));
    new_inode->reference_count = 1;
    new_inode->inode_number = dinodeid;
    new_inode->disk_inode_number = dinodeid;

    new_inode->mode = disk_inode.mode;
    new_inode->owner_uid = disk_inode.owner_uid;
    new_inode->owner_gid = disk_inode.owner_gid;
    new_inode->file_size = disk_inode.file_size;
    memcpy(new_inode->block_addresses, disk_inode.block_addresses, sizeof(disk_inode.block_addresses));

    // 4. 插入哈希链表
    new_inode->next = hinode[inodeid].prev_inode;
    new_inode->prev = nullptr;

    if (hinode[inodeid].prev_inode)
        hinode[inodeid].prev_inode->prev = new_inode;

    hinode[inodeid].prev_inode = new_inode;

    return new_inode;
}

/* 函数名：	iput							*/
/* 作用：	回收内存i结点					*/
/* 参数:	指向待回收的内存i结点指针		*/
/* 返回值：	无								*/
void iput(struct MemoryINode *pinode)
{
    long addr;

    // 1. 如果引用计数大于 1，只减少引用计数
    if (pinode->reference_count > 1)
    {
        pinode->reference_count--;
        return;
    }

    // 2. 如果引用计数等于 1，将内存 i-node 写回磁盘
    if (pinode->reference_count == 1)
    {
        addr = DISK_INODE_START_POINTOR + pinode->disk_inode_number * DISK_INODE_SIZE;

        // 构建磁盘 i-node
        DiskINode disk_inode;
        disk_inode.mode = pinode->mode;
        disk_inode.owner_uid = pinode->owner_uid;
        disk_inode.owner_gid = pinode->owner_gid;
        disk_inode.file_size = pinode->file_size;
        memcpy(disk_inode.block_addresses, pinode->block_addresses, sizeof(disk_inode.block_addresses));

        // 写回磁盘
        memcpy(disk + addr, &disk_inode, sizeof(DiskINode));
    }
    else if (pinode->reference_count == 0) // 3. 如果引用计数为 0，释放磁盘资源
    {
        // 释放数据块
        uint32_t block_num = (pinode->file_size + BLOCK_SIZE - 1) / BLOCK_SIZE; // 向上取整
        for (uint32_t i = 0; i < block_num; i++)
        {
            bfree(pinode->block_addresses[i]);
        }

        // 释放 i-node 号
        ifree(pinode->disk_inode_number);
    }

    // 4. 从哈希链表中删除内存 i-node
    int inodeid = pinode->disk_inode_number % NHINO;

    if (hinode[inodeid].prev_inode == pinode) // 如果是链表头
    {
        hinode[inodeid].prev_inode = pinode->next;
    }
    else // 链表中的其他位置
    {
        if (pinode->next)
        {
            if (pinode->prev)
                pinode->next->prev = pinode->prev;
            else
                pinode->next->prev = nullptr;
        }

        if (pinode->prev)
        {
            if (pinode->next)
                pinode->prev->next = pinode->next;
            else
                pinode->prev->next = nullptr;
        }
    }

    // 5. 释放内存 i-node
    free(pinode);
}

MemoryINode *get_parent_inode(MemoryINode *current_inode)
{
    DirectoryEntry entries[BLOCK_SIZE / sizeof(DirectoryEntry)];

    // 读取当前目录的第一个数据块
    memcpy(entries, disk + DATA_START_POINTOR + current_inode->block_addresses[0] * BLOCK_SIZE, sizeof(entries));

    // 遍历目录项，找到 '..' 对应的 inode
    for (int i = 0; i < BLOCK_SIZE / sizeof(DirectoryEntry); i++)
    {
        if (strcmp(entries[i].name, "..") == 0) // 找到父目录项
        {
            return iget(entries[i].inode_number); // 返回父目录的内存 inode
        }
    }

    return nullptr; // 如果未找到，返回空指针
}

MemoryINode *path_to_inode(const char *path)
{
    if (!path || strlen(path) == 0)
    {
        std::cerr << "错误: 空路径无效！" << std::endl;
        return nullptr;
    }

    // 起始目录: 根目录或当前目录
    MemoryINode *current_inode = (path[0] == '/') ? iget(1) : cwd;
    std::string resolved_path;
    // 解析路径
    if (path[0] == '/')
    {
        resolved_path = path;
    }
    else
    {
        std::string current_path = get_current_path();
        resolved_path = PathResolver::resolve(current_path, path);
    }
    std::vector<std::string> path_tokens = PathResolver::splitAndNormalize(resolved_path);

    for (const std::string &name : path_tokens)
    {
        if (!(current_inode->mode & DIDIR))
        {
            std::cerr << "错误: '" << name << "' 不是一个目录！" << std::endl;
            return nullptr;
        }

        bool found = false;

        // 遍历当前目录的数据块
        for (uint32_t block_idx = 0; block_idx < ADDRESS_POINTOR_NUM; ++block_idx)
        {
            if (current_inode->block_addresses[block_idx] == 0)
                break;

            uint32_t entries_per_block = BLOCK_SIZE / sizeof(DirectoryEntry);
            DirectoryEntry entries[entries_per_block];

            // 读取数据块
            memcpy(entries, disk + DATA_START_POINTOR + current_inode->block_addresses[block_idx] * BLOCK_SIZE, sizeof(entries));

            for (uint32_t i = 0; i < entries_per_block; ++i)
            {
                // 查找目标路径名
                if (entries[i].inode_number != 0 && strcmp(entries[i].name, name.c_str()) == 0)
                {
                    current_inode = iget(entries[i].inode_number);
                    found = true;
                    break;
                }
            }
            if (found)
                break;
        }

        if (!found)
        {
            std::cerr << "错误: 路径 '" << name << "' 不存在！" << std::endl;
            return nullptr;
        }
    }

    return current_inode;
}
