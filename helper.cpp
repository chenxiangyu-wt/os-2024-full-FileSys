#include <stdio.h>
#include <iostream>
#include <iomanip>
#include "dEntry.hpp"
#include "file_sys.hpp"

void printDirectoryBuffer(DirectoryEntry dir_buf[], int count)
{
    std::cout << "====== Directory Entries ======\n";
    std::cout << std::left << std::setw(20) << "Name" << std::setw(10) << "Inode" << "\n";
    std::cout << "----------------------------------\n";

    for (int i = 0; i < count; ++i)
    {
        if (dir_buf[i].inode_number != 0) // 忽略未使用的目录项
        {
            std::cout << std::left << std::setw(20) << dir_buf[i].name
                      << std::setw(10) << dir_buf[i].inode_number << "\n";
        }
    }
    std::cout << "================================\n";
}

void printDiskBlock(const uint8_t *disk, int start_offset, int block_size)
{
    std::cout << "===== Disk Block (Offset: " << start_offset << ", Size: " << block_size << " bytes) =====\n";

    for (int i = 0; i < block_size; ++i)
    {
        // 按十六进制输出每个字节
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (static_cast<int>(static_cast<unsigned char>(disk[start_offset + i]))) << " ";

        // 每 16 字节换行
        if ((i + 1) % 16 == 0)
            std::cout << "\n";
    }

    std::cout << "===========================================================\n";
}
void printDiskAsDirectory(const uint8_t *disk, int start_offset, int block_size)
{
    int entry_count = block_size / sizeof(DirectoryEntry);
    DirectoryEntry *entries = (DirectoryEntry *)(disk + start_offset);

    std::cout << "===== Disk Block as Directory Entries =====\n";
    std::cout << std::left << std::setw(20) << "Name" << std::setw(10) << "Inode Number" << "\n";
    std::cout << "--------------------------------------------\n";

    for (int i = 0; i < entry_count; ++i)
    {
        if (entries[i].inode_number != 0) // 过滤无效目录项
        {
            std::cout << std::left << std::setw(20)
                      << std::string(entries[i].name, ENTRY_NUM) // 确保长度限定
                      << std::setw(10) << entries[i].inode_number << "\n";
        }
    }
    std::cout << "===========================================\n";
}

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