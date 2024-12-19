# README

## 项目简介

`FileSys`是一个模拟文件系统的项目，旨在实现一个简单的文件系统，支持基本的文件操作和目录管理。本项目是为学习和实践文件系统原理而设计的，使用 C++ 编写，并涵盖了文件操作、路径解析、安全性、日志管理等模块。

## 项目结构

以下是项目目录结构：

```
.
├── archive                # 存放归档相关的文件或代码
├── command_line.hpp       # 处理命令行输入的头文件
├── command_line.cpp       # 处理命令行输入的实现文件
├── dEntry.hpp             # 定义目录项结构的头文件
├── dEntry.cpp             # 目录项操作的实现文件
├── direction.cpp          # 用于处理文件方向的代码
├── file.cpp               # 文件操作实现
├── file_sys.hpp           # 文件系统类的声明
├── file_sys.cpp           # 文件系统类的实现
├── globals.hpp            # 全局常量、变量声明
├── helper.cpp             # 辅助函数的实现
├── helper.hpp             # 辅助函数的声明
├── iNode.hpp              # iNode（索引节点）结构声明
├── iNode.cpp              # iNode相关操作实现
├── log.cpp                # 日志记录模块实现
├── log.hpp                # 日志记录模块声明
├── main.cpp               # 程序入口
├── Makefile               # 构建工具文件
├── PathResolver.hpp       # 路径解析的头文件
├── PathResolver.cpp       # 路径解析的实现文件
├── README.md              # 项目的 README 文档
├── security.cpp           # 安全性相关功能实现
├── security.hpp           # 安全性相关功能声明
├── utils.cpp              # 工具函数实现
├── utils.hpp              # 工具函数声明
```

核心模块

1. FileSys
   file_sys.hpp 和 file_sys.cpp 文件包含了文件系统的核心类，负责文件和目录的管理。此模块实现了文件系统的基本操作，如创建、删除文件，查询文件信息等。

2. iNode
   iNode.hpp 和 iNode.cpp 实现了索引节点的管理。iNode 是文件系统中用来存储文件元数据（如文件大小、权限、数据块位置等）的结构。

3. 命令行解析
   command_line.hpp 和 command_line.cpp 提供了命令行接口解析功能，允许用户输入指令来与文件系统交互。

4. 路径解析
   PathResolver.hpp 和 PathResolver.cpp 用于解析文件路径，帮助将文件路径转化为文件系统能够理解的结构。

5. 日志
   log.hpp 和 log.cpp 实现了简单的日志记录功能，记录系统的关键操作和错误信息。

6. 安全性
   security.hpp 和 security.cpp 提供了文件访问控制和安全性功能，确保只有授权用户能够访问特定文件或目录。

7. 工具函数
   utils.hpp 和 utils.cpp 包含了项目中使用的常用工具函数，旨在减少重复代码，提高开发效率。

## 使用方法

### 编译

```shell
make
```

### 运行

```shell
./out/file_sys
```
