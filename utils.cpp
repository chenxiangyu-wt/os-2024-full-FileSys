#include <iostream>
#include <iomanip>
#include <cstdint>
#include "utils.hpp"
#include "file_sys.hpp"
#include "globals.hpp"

void printDisk()
{
    const int totalBlocks = DISK_SIZE / BLOCK_SIZE; // 总块数

    std::cout << "===== Simulated Disk Dump (Block Size: " << BLOCK_SIZE
              << " bytes, Total Blocks: " << totalBlocks << ") =====" << std::endl;

    for (int blockIndex = 0; blockIndex < totalBlocks; ++blockIndex)
    {
        // 计算块的起始偏移
        int offset = blockIndex * BLOCK_SIZE;

        // 显示块信息
        std::cout << "Block " << blockIndex << " (Offset: 0x"
                  << std::setw(8) << std::setfill('0') << std::hex << offset << "):\n";

        // 打印块内容
        for (int i = 0; i < BLOCK_SIZE; ++i)
        {
            std::cout << std::setw(2) << std::setfill('0') << std::hex
                      << static_cast<int>(disk[offset + i]) << " ";

            // 每 16 字节换一行
            if ((i + 1) % 16 == 0)
            {
                std::cout << std::endl;
            }
        }
        std::cout << "====================================================\n";
    }

    std::cout << "===== End of Disk Dump =====" << std::endl;
}