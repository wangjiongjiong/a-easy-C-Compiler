#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#define int long long

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


// 标记,类型 ，操作符在最后，按优先顺序排列
enum {
  Num = 128, Fun, Sys, Glo, Loc, Id,
  Char, Else, Enum, If, Int, Return, Sizeof, While,
  Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, 
  Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak
};

/* 符号表定义，采用数组的方式存储，数组大小为9 */
int token_val;         //当前token的值，用整数来表示
int *current_id,       //当前的id
    *symbols;          //符号表指针

// 标识符字段，也就是符号表字段
enum {Token, Hash, Name, Type, Class, Value, BType, BClass, BValue, IdSize};


// 函数变量类型
enum { CHAR, INT, PTR };
int *idmain;           //主函数指针



/* 词法分析模块 */
void next()
{
    /* token = *src++;
    return; */
    char *last_pos;
    int hash;

    while (token = *src)
    {
        ++src;
        //解析 token;
        if(token == '\n')
        {
            ++line;
        }
        else if (token == '#')
        {
            //跳过宏定义，编译器不持支宏定义
            while(*src != 0 && *src != '\n')
            {
                src++;
            }
        }
        else if ((token >= 'a' && token <= 'z') || 
        (token >= 'A' && token <= 'Z') || (token == '_')) 
        {

            // 解析标识符
            last_pos = src - 1;
            hash = token;

            while ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') 
            || (*src >= '0' && *src <= '9') || (*src == '_')) 
            {
                hash = hash * 147 + *src;
                src++;
            }

            // 寻找现有的标识符，线性查找,有点理解了，但是如果自己复现还是会有点困难
            //第一次接触编译器开发重要还是入门理解为主
            current_id = symbols;
            while (current_id[Token]) 
            {
                //判断当先的hash值是否与符号表中存储的一致，并且判断name字段是否相等
                if (current_id[Hash] == hash && !memcmp((char *)current_id[Name], 
                last_pos, src - last_pos)) 
                {
                    //找到一个已有的标识符就返回，看不懂寄寄
                    token = current_id[Token];
                    return;
                }
                //相当于找到符号表的下一个符号
                current_id = current_id + IdSize;
            }

            // 存储新的id
            current_id[Name] = (int)last_pos;
            current_id[Hash] = hash;
            token = current_id[Token] = Id;
            return;
        }
        else if (token >= '0' && token <= '9') 
        {
            //针对三种进制进行解析，10进制(123) 16进制(0x123) 8进制(017)
            token_val = token - '0';
            if (token_val > 0) 
            {
                // 10进制开始于1-9
                while (*src >= '0' && *src <= '9') 
                {
                    token_val = token_val*10 + *src++ - '0';
                }
            } 
            else 
            {
                // 开始字符为0
                if (*src == 'x' || *src == 'X') 
                {
                    //16进制
                    token = *++src;
                    while ((token >= '0' && token <= '9') || (token >= 'a' && token <= 'f') 
                    || (token >= 'A' && token <= 'F')) 
                    {
                        token_val = token_val * 16 + (token & 15) + (token >= 'A' ? 9 : 0);
                        token = *++src;
                    }
                } 
                else 
                {
                    // 8进制
                    while (*src >= '0' && *src <= '7') 
                    {
                        token_val = token_val*8 + *src++ - '0';
                    }
                }
            }

            token = Num;
            return;
        }
        else if (token == '"' || token == '\'') 
        {
            // 解析字符串文字，目前唯一支持的转义
            // 字符为“\n”，将字符串文字存储到数据中。
            last_pos = data;
            while (*src != 0 && *src != token) 
            {
                token_val = *src++;
                if (token_val == '\\') 
                {
                    // 转义字符，这里我们只支持了\n这一种转义字符
                    token_val = *src++;
                    if (token_val == 'n') 
                    {
                        token_val = '\n';
                    }
                }

                if (token == '"') 
                {
                    *data++ = token_val;
                }
            }

            src++;
            // 如果是单个字符，则返回 Num token
            if (token == '"') 
            {
                token_val = (int)last_pos;
            } else 
            {
                token = Num;
            }

            return;
        }
        else if (token == '/') 
        {
            if (*src == '/') 
            {
                // 跳过
                while (*src != 0 && *src != '\n') 
                {
                    ++src;
                }
            } 
            else 
            {
                // 除法符号
                token = Div;
                return;
            }
        }
        else if (token == '=') 
        {
            // 解析==和=
            if (*src == '=') 
            {
                src ++;
                token = Eq;
            } 
            else 
            {
                token = Assign;
            }
            return;
        }
        else if (token == '+') 
        {
            // 解析++和+
            if (*src == '+') 
            {
                src ++;
                token = Inc;
            } 
            else 
            {
                token = Add;
            }
            return;
        }
        else if (token == '-') 
        {
            // 解析--和-
            if (*src == '-') 
            {
                src ++;
                token = Dec;
            } 
            else 
            {
                token = Sub;
            }
            return;
        }
        else if (token == '!') 
        {
            // 解析 !=
            if (*src == '=') 
            {
                src++;
                token = Ne;
            }
            return;
        }
        else if (token == '<') 
        {
            // 解析<=,<<,<
            if (*src == '=') 
            {
                src ++;
                token = Le;
            } 
            else if (*src == '<') 
            {
                src ++;
                token = Shl;
            } 
            else 
            {
                token = Lt;
            }
            return;
        }
        else if (token == '>') 
        {
            // 解析>=,>>,>
            if (*src == '=') 
            {
                src ++;
                token = Ge;
            } 
            else if (*src == '>') 
            {
                src ++;
                token = Shr;
            } else 
            {
                token = Gt;
            }
            return;
        }
        else if (token == '|') 
        {
            // 解析||和|
            if (*src == '|') 
            {
                src ++;
                token = Lor;
            } 
            else 
            {
                token = Or;
            }
            return;
        }
        else if (token == '&') 
        {
            // 接卸&&和&
            if (*src == '&') 
            {
                src ++;
                token = Lan;
            } 
            else 
            {
                token = And;
            }
            return;
        }
        else if (token == '^') 
        {
            token = Xor;
            return;
        }
        else if (token == '%') 
        {
            token = Mod;
            return;
        }
        else if (token == '*') 
        {
            token = Mul;
            return;
        }
        else if (token == '[') 
        {
            token = Brak;
            return;
        }
        else if (token == '?') 
        {
            token = Cond;
            return;
        }
        else if (token == '~' || token == ';' || token == '{' || token == '}' 
        || token == '(' || token == ')' || token == ']' || token == ',' || token == ':') 
        {
            // 这些字符直接返回作为token
            return;
        }

    }
    
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
        op = *pc++;
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
            //将ax的值放入栈中
            *--sp = ax;
        }
        else if (op == JMP)
        {
            //无条件跳转地址，将pc中的值设为地址
            pc = (int *)*pc;
        }
        else if (op == JZ)
        {
            //判断ax中的值是否为0若为0则跳转到指定地址
            pc = ax ? pc + 1 : (int *)*pc;
        }
        else if (op == JNZ)
        {
            //判断ax中的值是否不为0若不为0则跳转到指定地址
            pc = ax ? (int*)*pc : pc + 1; 
        }
        else if (op == CALL)
        {
            //跳转到子函数的地址
            *--sp = (int)(pc+1);
            pc = (int *)*pc;
        }
        else if (op == ENT)
        {
            //保存当前栈指针，并在栈上保留一定的空间
            *--sp = (int)bp;
            bp = sp;
            sp = sp - *pc++;
        }
        else if (op == ADJ)
        {
            //清除子函数入栈的数据
            sp = sp + *pc++;
        }
        else if (op == LEV)
        {
            //还原原来函数的pc等信息
            sp = bp;
            bp = (int *)sp++;
            pc = (int *)sp++;
        }
        else if (op == LEA)
        {
            ax = (int)(bp + *pc++);
        }
        else if (op == OR)
        {
            ax = *sp++ | ax;
        }
        else if (op == XOR)
        {
            ax = *sp++ ^ ax;
        }
        else if (op == AND)
        {
            ax = *sp++ & ax;
        }
        else if (op == EQ)
        {
            ax = *sp++ == ax;
        }
        else if (op == NE)
        {
            ax = *sp++ != ax;
        }
        else if (op == LT)
        {
            ax = *sp++ < ax;
        }
        else if (op == LE)
        {
            ax = *sp++ <= ax;
        }
        else if (op == GT)
        {
            ax = *sp++ > ax;
        }
        else if (op == GE)
        {
            ax = *sp++ >= ax;
        }
        else if (op == SHL)
        {
            ax = *sp++ << ax;
        }
        else if (op == SHR)
        {
            ax = *sp++ >> ax;
        }
        else if (op == ADD)
        {
            ax = *sp++ + ax;
        }
        else if (op == SUB)
        {
            ax = *sp++ - ax;
        }
        else if (op == MUL)
        {
            ax = *sp++ * ax;
        }
        else if (op == DIV)
        {
            ax = *sp++ / ax;
        }
        else if (op == MOD)
        {
            ax = *sp++ % ax;
        }
        else if (op == EXIT)
        {
            //退出时打印sp
            printf("exit(%d)",*sp);
            return *sp;
        }
        else if (op == OPEN)
        {
            //打开源文件
            ax = open((char *)sp[1],sp[0]);
        }
        else if (op == CLOS)
        {
            //关闭源文件
            ax = close(*sp);
        }
        else if (op == READ)
        {
            //读取源文件
            ax = read(sp[2],(char *)sp[1],*sp);
        }
        else if (op == PRTF)
        {
            //printf函数
            tmp = sp + pc[1];
            ax = printf((char *)tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5], tmp[-6]);
        }
        else if (op == MALC)
        {
            //malloc函数
            ax = (int)malloc(*sp);
        }
        else if (op == MSET)
        {
            //memset函数
            ax = (int)memset((char *)sp[2], sp[1], *sp);
        }
        else if (op == MCMP)
        {
            //memcmp函数
            ax = memcmp((char *)sp[2], (char *)sp[1], *sp);
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

    src = "char else enum if int return sizeof while "
          "open read close printf malloc memset memcmp exit void main";

     // 将关键字添加到符号表
    i = Char;
    while (i <= While) {
        next();
        current_id[Token] = i++;
    }

    // 将库添加到符号表
    i = OPEN;
    while (i <= EXIT) {
        next();
        current_id[Class] = Sys;
        current_id[Type] = INT;
        current_id[Value] = i++;
    }

    next(); current_id[Token] = Char; // 处理空类型
    next(); idmain = current_id; // 跟踪main函数

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
    
    /* i = 0;
    text[i++] = IMM;
    text[i++] = 10;
    text[i++] = PUSH;
    text[i++] = IMM;
    text[i++] = 20;
    text[i++] = ADD;
    text[i++] = PUSH;
    text[i++] = EXIT;
    pc = text; */ 

    program();
    return eval();
}