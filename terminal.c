#include "terminal.h"
#include "cmd_help.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

int set_tty_mode(void)
{
    struct termios new_mode;
    //获取当前终端参数信息
    if( !0 == tcgetattr(0, &new_mode))
    {
        perror("tcgetattr error");
        return -1;
    }
    //去除标准输入模式
    new_mode.c_lflag &= ~ICANON;
    //关闭回显功能
    new_mode.c_lflag &= ~ECHO;
    //非规范模式下读取一个字符
    new_mode.c_cc[VMIN] = 1;
    if(-1 == tcsetattr(0, TCSANOW, &new_mode) )
    {
        perror("tcstattr error");
        return -1;
    }
    return 0;
}

void usart_putchar(const char* const ptr)
{
    putc(*ptr, stdout);
}

char usart_getchar(void)
{
    set_tty_mode();
    return getchar();
}

int register_cmd(char *name, char *usage, cmd_fun_t fun)
{
    int ret;
    if (cmd_num_current < MAX_CMD_NUM)
    {
        CmdTbl[cmd_num_current].name = name;
        CmdTbl[cmd_num_current].usage = usage;
        CmdTbl[cmd_num_current].CmdFun = fun;
        cmd_num_current++;
    }
    else
    {
        printf("%s error\n");
        return 1;
    }
    return 0;
}

char parse_buf[256] ;
int parse_line(const char* const line, char* argv[])
{
    int argc = 0;
    char *ptr = parse_buf;
    memset(parse_buf, '\0', 256);
    strncpy(parse_buf, line, strlen(line));

    while ((argv[argc]=strtok(ptr, " "))!=NULL)
    {
        //printf("argv[%d]:%s\n", argc, argv[argc]);
        argc++;
        if (argc > MAX_ARGC)
            break;
        ptr = NULL;
    }
    return argc;
}

static char * delete_char (char *buffer, char *p, int *colp, int *np, int plen)
{
    char *s;
    if (*np == 0)
    {
        return (p);
    }

    if (*(--p) == '\t') {           /* will retype the whole line   */
        while (*colp > plen) {
            puts (erase_seq);
            (*colp)--;
        }
        for (s=buffer; s<p; ++s) {
            if (*s == '\t') {
                puts (tab_seq+((*colp) & 07));
                *colp += 8 - ((*colp) & 07);
            } else {
                ++(*colp);
                putc (*s, stdout );
            }
        }
    } else {
        puts (erase_seq);
        (*colp)--;
    }
    (*np)--;
    return (p);
}

void print_prompt(const char* prompt)
{
    int prompt_len;
    char* ptr = prompt;

    if (prompt)
    {
        prompt_len = strlen (prompt);
        while (*ptr)
        {
            usart_putchar(ptr);
            ptr++;
        }
    }
}


int32_t readline_into_buffer ( char* const prompt, char* buffer)
{
    char ch;
    int  n = 0;          /* input  buffer index */
    int  col;            /* output column cnt   */
    int prompt_len;      /* prompt length       */

    char *p = buffer;    /*input buff*/
    char *p_buf = p;

    print_prompt(prompt);

    prompt_len=strlen(prompt);
    col = prompt_len;

    while (1)
    {
        ch = usart_getchar();
        printf("%c",ch);//commment in stm32
        switch (ch)
        {
            case '\r':              /* Enter        */
            case '\n':
                *p = '\0';
                return (p - p_buf);

            case '\t':
                find_similar_cmd(buffer);
                return -1;

            case '\0':              /* null          */
                continue;

            case 0x03:              /* ^C - break       */
                p_buf[0] = '\0';    /* discard input */
                continue;

            case 0x15:              /* ^U - erase line  */
                while (col > prompt_len)
                {
                    puts (erase_seq);
                    --col;
                }
                p = p_buf;
                n = 0;
                continue;

            case 0x17:              /* ^W - erase word  */
                p=delete_char(p_buf, p, &col, &n, prompt_len);
                while ((n > 0) && (*p != ' '))
                {
                    p=delete_char(p_buf, p, &col, &n, prompt_len);
                }
                continue;

            case 0x08:              /* ^H  - backspace  */
            case 0x7F:              /* DEL - backspace  */
                p=delete_char(p_buf, p, &col, &n, prompt_len);
                continue;

            default:
                /*
                 * Must be a normal character then
                 */
                if (n < CONFIG_SYS_CBSIZE-2)
                {
                    ++ col;/*  echo input  */
                    *p++ = ch;
                    ++n;
                }
                else/*   Buffer full   */
                {
                    usart_putchar ('\a');
                }
        }
    }
}


int32_t readline (char *const prompt)
{
    memset(console_buffer, '\0', CONFIG_SYS_CBSIZE+1);
    console_buffer[0] = '\0';

    return readline_into_buffer(prompt, console_buffer);
}

int find_similar_cmd(char* cmd)
{
    int cmd_index = 0;
    int similar_cmd_count = 0;
    int similar_cmd_item[MAX_CMD_NUM]={0};

    while (cmd_index < MAX_CMD_NUM)
    {
        if(CmdTbl[cmd_index].name == NULL)  break;
        if (strncmp(CmdTbl[cmd_index].name, cmd, strlen(cmd)) == 0)
        {
            if(similar_cmd_count > MAX_CMD_NUM) break;
            similar_cmd_item[similar_cmd_count] = cmd_index;
            ++similar_cmd_count;
        }
        cmd_index++;
    }
    printf("\r\n");
    for(int i=0;i<similar_cmd_count;i++)
    {
        printf("[%d]%s\r\n",i,CmdTbl[similar_cmd_item[i]].name);
    }
    return 0;
}

int find_cmd(char* cmd)
{
    int cmd_index = 0;
    int similar_cmd_count = 0;
    int similar_cmd_item[MAX_CMD_NUM]={0};

    while (cmd_index < MAX_CMD_NUM)
    {
        if(CmdTbl[cmd_index].name == NULL)  break;
        if (strncmp(CmdTbl[cmd_index].name, cmd, strlen(cmd)) == 0)
        {
            if(strcmp(cmd,CmdTbl[cmd_index].name)==0)
            {
                similar_cmd_count =1;
                similar_cmd_item[0] = cmd_index;
                break;
            }
            if(similar_cmd_count > MAX_CMD_NUM) break;
            similar_cmd_item[similar_cmd_count] = cmd_index;
            ++similar_cmd_count;
        }
        cmd_index++;
    }

    switch(similar_cmd_count)
    {
        case 0:{
            printf("Command  [%s ]  don't support!\n", cmd);
            return -1;
        }
        case 1:{
            return similar_cmd_item[0];
        }
        default:{
            for(int i=0;i<similar_cmd_count;i++)
            {
                printf("[%d]%s\r\n",i,CmdTbl[similar_cmd_item[i]].name);
            }
        }
    }

    return -1;
}

int run_command (const char * const cmd)
{
    int cmd_index = 0;
    int argc = 0;
    char *argv[MAX_ARGC];

    argc = parse_line(cmd, argv);

    if ((argc > 0) && (argc < MAX_ARGC))
    {
        cmd_index = find_cmd(argv[0]);
    }
    else
    {
        return 1;
    }

    if (cmd_index != -1)
    {
        CmdTbl[cmd_index].CmdFun(argc, argv);
    }
    return 0;
}

void terminal_init()
{

}

void terminal_spin()
{
    int32_t rc = -1;
    int32_t len;
    static char lastcommand[CONFIG_SYS_CBSIZE] = { 0 };
    memset(CmdTbl, 0, sizeof(CMD_STRUCT_T)*MAX_CMD_NUM);

    register_cmd("help",  "help0\n\r", HelpCmdExeFun);
    register_cmd("help1", "help1\n\r", HelpCmdExeFun);
    register_cmd("help2", "help2\n\r", HelpCmdExeFun);
    register_cmd("heel", "hell all cmd\n\r", HelpCmdExeFun);
    register_cmd("helf", "helf all cmd\n\r", HelpCmdExeFun);
    register_cmd("helq", "helq all cmd\n\r", HelpCmdExeFun);
    register_cmd("hdff", "heeff all cmd\n\r", HelpCmdExeFun);
    register_cmd("hcdd", "hedddd all cmd\n\r", HelpCmdExeFun);
    register_cmd("hblp2", "heebb all cmd\n\r", HelpCmdExeFun);
    register_cmd("hblp1", "heelff all cmd\n\r", HelpCmdExeFun);

    while (1)
    {
        len = readline (CONFIG_SYS_PROMPT);
        if (len > 0)
        {
            memset(lastcommand, '\0', CONFIG_SYS_CBSIZE);
            strncpy (lastcommand, console_buffer, strlen(console_buffer));

            rc = run_command (lastcommand);
            if (rc <= 0)
            {
                /* invalid command or not repeatable, forget it */
                lastcommand[0] = 0;
            }
        }
    }
}

