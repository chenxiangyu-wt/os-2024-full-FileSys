# 编译器
CC = g++

# 编译选项
CFLAGS = -Wall -Wextra -std=c++11

# 项目名
TARGET = out/filesys

# 源文件列表
SRCS = $(wildcard *.cpp)

# 对应的目标文件列表
OBJS = $(patsubst %.cpp, out/%.o, $(SRCS))

# 默认目标
all: $(TARGET)

# 链接生成可执行文件
$(TARGET): $(OBJS)
	@mkdir -p out
	$(CC) $(CFLAGS) -o $@ $^

# 编译 .cpp 文件生成 .o 文件
out/%.o: %.cpp
	@mkdir -p out
	$(CC) $(CFLAGS) -c $< -o $@

# 清理生成的文件
clean:
	rm -rf out

# 伪目标（不检查文件同名情况）
.PHONY: all clean