#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

int token;              //当前的token
char *src , *old_src;   //指向源代码字符串的指针
int poolsize;           //默认文本，数据，栈的大小
int line;               //line number


void next()
{
    token = *src++;
    return;
}

void expression(int level) {
    // do nothing
}

void program() {
    next();                  // 获取到下一个token
    while (token > 0) {
        printf("token is: %c\n", token);
        next();
    }
}


int eval() { // do nothing yet
    return 0;
}

int main(int argc, char **argv)
{
    int i, fd;

    argc--;
    argv++;

    poolsize = 256 * 1024; // 任意大小即可
    line = 1;

    if ((fd = open(*argv, 0)) < 0) {
        printf("could not open(%s)\n", *argv);
        return -1;
    }

    if (!(src = old_src = malloc(poolsize))) {
        printf("could not malloc(%d) for source area\n", poolsize);
        return -1;
    }

    // 读取c语言源代码
    if ((i = read(fd, src, poolsize-1)) <= 0) {
        printf("read() returned %d\n", i);
        return -1;
    }
    src[i] = 0; // 加入 EOF符号
    close(fd);

    program();
    return eval();
}