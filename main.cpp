#include <QCoreApplication>
#include "uterminal.h"
#include<stdio.h>
void HelpCmdExeFun(int agrc, char *argv[])
{
    printf("hello ,this is help\r\n");
    int i=0;
    for(i=0;i < agrc;i++)
    {
        printf("argv[%d]:%s \r\n",i,argv[i]);
    }
    printf("\r\n");
    return;
}
int putc_(char byte)
{
    putc(byte,stdout);
    return 1;
}
int main(int argc, char *argv[])
{
    unsigned char ch;
    CMD_NCB_ST *fs;
    cmd_create(&fs,printf,putc_,"robot:");
    fs->printf("fs is %d\r\n",fs);
    fs->printf("fs is_initialize is %d\r\n",fs->is_initialize);
    fs->printf("fs is_initialize cmd_num_current is %d\r\n",fs->cmd_num_current);
    cmd_register(fs,"help",  "help0\n\r", HelpCmdExeFun);
    cmd_register(fs,"help1", "help1\n\r", HelpCmdExeFun);
    cmd_register(fs,"help2", "help2\n\r", HelpCmdExeFun);
    cmd_register(fs,"heel", "hell all cmd\n\r", HelpCmdExeFun);
    cmd_register(fs,"helf", "helf all cmd\n\r", HelpCmdExeFun);
    cmd_register(fs,"helq", "helq all cmd\n\r", HelpCmdExeFun);
    cmd_register(fs,"hdff", "heeff all cmd\n\r", HelpCmdExeFun);
    cmd_register(fs,"hcdd", "hedddd all cmd\n\r", HelpCmdExeFun);
    cmd_register(fs,"hblp2", "heebb all cmd\n\r", HelpCmdExeFun);
    cmd_register(fs,"hblp1", "heelff all cmd\n\r", HelpCmdExeFun);
    fs->printf("fs is_initialize cmd_num_current is %d\r\n",fs->cmd_num_current);
    while(1)
    {
        cmd_read_lines(fs,ch); //put this function in the usart irq
        scanf("%c",&ch);
    }
      return 1;
}
