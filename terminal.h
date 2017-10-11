#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MAX_ARGC 10
#define MAX_CMD_NUM 50

#define CONFIG_SYS_CBSIZE 256

#define CONFIG_SYS_PROMPT "robotics:~"

char console_buffer[CONFIG_SYS_CBSIZE + 1];  /* console I/O buffer   */
static char erase_seq[] = "\b \b";      /* erase sequence   */
static char   tab_seq[] = "        ";       /* used to expand TABs  */

typedef void (*cmd_fun_t)(int , char *[])  ;
typedef struct CMD_STRUCT
{
    char* name; /* Command Name */
    char* usage;/* Usage message */
    cmd_fun_t CmdFun;//void (*CmdFun)(int , char *[]);/* Command execute function */
}CMD_STRUCT_T;

CMD_STRUCT_T CmdTbl[MAX_CMD_NUM];

int cmd_num_current = 0;

void terminal_spin();

#endif // TERMINAL_H
