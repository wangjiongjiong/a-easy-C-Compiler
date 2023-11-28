#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

/* 初始化指针声明 */
int token;              //当前的token
char *src , *old_src;   //指向源代码字符串的指针
int poolsize;           //默认文本，数据，栈的大小
int line;               //line number

/* 程序段指针声明 */
int *text,              //代码段指针
    *old_text,          //已经抛弃的代码段指针
    *stack;             //栈指针
char *data;             //数据段

/* 虚拟机寄存器指针声明 */
int *pc,*bp,*sp,ax,cycle;

/* 指令集(有参数的放在枚举的前面，无参数的放在枚举的后面) */
enum { LEA ,IMM ,JMP ,CALL,JZ  ,JNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PUSH,
       OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,
       OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT };


/* 词法分析模块 */
void next()
{
    token = *src++;
    return;
}


/* 解析表达式 */
void expression(int level) 
{
    // do nothing
}

/* 语法分析模块 */
void program() 
{
    next();                  // 获取到下一个token
    while (token > 0) 
    {
        printf("token is: %c\n", token);
        next();
    }
}

/* 虚拟机模块 */
int eval() 
{ 
    int op, *tmp;
    while (1)
    {
        if(op == IMM)
        {
            //存储一个立即数
            ax = *pc++;
        }
        else if (op == LC)
        {
            //存储字符到ax中，ax存储地址
            ax = *(char *)ax;
        }
        else if (op == LI)
        {
            //存储整数到ax中，ax存储地址
            ax = *(int *)ax;
        }else if (op == SC)
        {
            //保存ax中的字符到栈顶
            ax = *(char *)*sp++ = ax;
        }
        else if (op == SI)
        {
            //保存ax中的整数到栈顶
            *(int *)*sp++ = ax;
        }
        else if (op == PUSH)
        {
            /* code */
        }
        else if (op == JMP)
        {
            /* code */
        }
        else if (op == JZ)
        {
            /* code */
        }
        else if (op == JNZ)
        {
            /* code */
        }
        else if (op == ENT)
        {
            /* code */
        }
        else if (op == ADJ)
        {
            /* code */
        }
        else if (op == LEA)
        {
            /* code */
        }
        else if (op == LEA)
        {
            /* code */
        }
        else if (op == OR)
        {
            /* code */
        }
        else if (op == XOR)
        {
            /* code */
        }
        else if (op == AND)
        {
            /* code */
        }
        else if (op == EQ)
        {
            /* code */
        }
        else if (op == NE)
        {
            /* code */
        }
        else if (op == LT)
        {
            /* code */
        }
        else if (op == LE)
        {
            /* code */
        }
        else if (op == GT)
        {
            /* code */
        }
        else if (op == GE)
        {
            /* code */
        }
        else if (op == SHL)
        {
            /* code */
        }
        else if (op == SHR)
        {
            /* code */
        }
        else if (op == ADD)
        {
            /* code */
        }
        else if (op == SUB)
        {
            /* code */
        }
        else if (op == MUL)
        {
            /* code */
        }
        else if (op == DIV)
        {
            /* code */
        }
        else if (op == MOD)
        {
            /* code */
        }
        else if (op == EXIT)
        {
            /* code */
        }
        else if (op == OPEN)
        {
            /* code */
        }
        else if (op == CLOS)
        {
            /* code */
        }
        else if (op == READ)
        {
            /* code */
        }
        else if (op == PRTF)
        {
            /* code */
        }
        else if (op == MALC)
        {
            /* code */
        }
        else if (op == MSET)
        {
            /* code */
        }
        else if (op == MCMP)
        {
            /* code */
        }
        else
        {
            printf("unknow instruction%d\n",op);
            return -1;
        }
          
    }
    
    return 0;
}

int main(int argc, char **argv)
{
    int i, fd;

    argc--;
    argv++;

    poolsize = 256 * 1024; // 任意大小即可
    line = 1;



    // 为虚拟机分配内存
    if (!(text = old_text = malloc(poolsize))) 
    {
        printf("could not malloc(%d) for text area\n", poolsize);
        return -1;
    }
    if (!(data = malloc(poolsize))) 
    {
        printf("could not malloc(%d) for data area\n", poolsize);
        return -1;
    }
    if (!(stack = malloc(poolsize))) 
    {
        printf("could not malloc(%d) for stack area\n", poolsize);
        return -1;
    }

    /* memset函数是c语言中一个初始化函数，
    作用是将某一块内存中的全部设置为指定的值。
    这里就是将代码段、数据段、栈分配好内存
    并且将他们的初始值全都设置为0 */
    memset(text, 0, poolsize);
    memset(data, 0, poolsize);
    memset(stack, 0, poolsize);

    /* 寄存器指针初始化 */
    /* 这里这样初始化的目的是，我们已经知道了计算机的栈
    其实是从高地址向低地址入栈的因此我们bp和sp指针最开始应该指向
    栈的最高地址，因此这样初始化。 */
    bp = sp = (int *)((int)stack + poolsize);
    ax = 0;

    /* open打开想要读取的源代码 */
    if ((fd = open(*argv, 0)) < 0) 
    {
        printf("could not open(%s)\n", *argv);
        return -1;
    }

    /* malloc出来源代码的数据缓冲区 */
    if (!(src = old_src = malloc(poolsize))) 
    {
        printf("could not malloc(%d) for source area\n", poolsize);
        return -1;
    }

    // 读取c语言源代码
    if ((i = read(fd, src, poolsize-1)) <= 0) 
    {
        printf("read() returned %d\n", i);
        return -1;
    }
    src[i] = 0; // 加入 EOF符号
    close(fd);

    program();
    return eval();
}