SRCS        = $(wildcard *.c ./src/*.c)    # wildcard把指定目录./和 ../src下的所有后缀是c的文件全部展开
OBJS        = $(SRCS:.c = .o)    #OBJS将$(SRCS)下的.c文件转化为.o文件
CC          = gcc
CPP         = g++
INCLUDES    = -I./include
LIBS        = -L./lib
CPPFLAGS    = -ggdb3  -Werror -Wextra -std=c++11
OUTPUT      = main

all:$(OUTPUT)
$(OUTPUT) : $(OBJS)
	$(CC) $^ -o $@ $(INCLUDES) $(LIBS)
%.o : %.c
	$(CC) -c $< $(CPPFLAGS)
run:
	./main
clean:
	@rm -rf main *.o
.PHONY:clean
