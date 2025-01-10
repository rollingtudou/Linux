# 编译器
CC = gcc

# 源文件
SRCS = $(wildcard *.c)

# 目标文件
OBJS = $(SRCS:.c=.o)

# 目标可执行文件
TARGET = exe

# 构建目标可执行文件的规则
$(TARGET): $(OBJS)
	$(CC) -o $@ $^

# 构建目标文件的规则
%.o: %.c
	$(CC) -c $< -o $@ -std=c99 -g

# 清理生成的文件
.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)
