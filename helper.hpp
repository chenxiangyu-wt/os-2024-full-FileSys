#ifndef HELPER_JPP
#define HELPER_HPP
#include <stdint.h>
#include "dEntry.hpp"
#include "file_sys.hpp"
void printDirectoryBuffer(DirectoryEntry dir_buf[], int count);
void printDiskBlock(const uint8_t *disk, int start_offset, int block_size);
void printDiskAsDirectory(const uint8_t *disk, int start_offset, int block_size);
void printFileSystem(const FileSystem &fs);
#endif // HELPER_HPP