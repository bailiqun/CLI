#ifndef UTERMINAL_H
#define UTERMINAL_H
#ifdef __cplusplus
extern "C" {
#endif

    #include <stdio.h>
    #include <stdlib.h>
    #include <stdint.h>
    #include <string.h>
    #ifndef NULL
        #define NULL 0
    #endif
    #ifndef TRUE
        #define TRUE 1
    #endif
    #ifndef FALSE
        #define FALSE 0
    #endif
    #define CMD_MAX_ARGC 10
    #define CMD_MAX_CMD_NUM 50
    #define CMD_CONFIG_SYS_CBSIZE 256

    static char erase_seq[] = "\b \b";      /* erase sequence   */
    static char   tab_seq[] = "        ";       /* used to expand TABs  */
    typedef void (*cmd_fun_t)(int , char *[])  ;
    typedef int (*printf_st)(const char *format,...);
    typedef int (*putc_st)(char byte);
    typedef struct CMD_STRUCT
    {
        char* name;
        char* usage;
        cmd_fun_t cmd_cb;
    }CMD_STRUCT_T;
    typedef struct CMD_NCB_ST
    {
        char is_initialize;

        CMD_STRUCT_T cmd_tbl[CMD_MAX_CMD_NUM];
        unsigned int cmd_num_current;

        putc_st    putc;
        printf_st   printf;

        unsigned int argc;
        char *argv[CMD_MAX_ARGC];
        char argv_buffer[CMD_CONFIG_SYS_CBSIZE + 1] ;
        char rx_buffer[CMD_CONFIG_SYS_CBSIZE  + 1];
        unsigned int rx_buffer_size;
        unsigned int  rx_buffer_cnt;

        const char *   prompt_buffer;
        unsigned int    prompt_size;
    }CMD_NCB_ST;
    extern  int cmd_get_one_cmd_from_buff(CMD_NCB_ST *fs, unsigned char data);
    extern  int cmd_destory(CMD_NCB_ST *fs);
    extern  int cmd_register(CMD_NCB_ST *fs, char *name, char *usage, cmd_fun_t fun);
    extern  int cmd_create(CMD_NCB_ST **pfs, printf_st printf_, putc_st putc_, const char * const prompt);
    extern  int cmd_read_lines(CMD_NCB_ST *fs, unsigned char data);
#ifdef __cplusplus
}
#endif
#endif // UTERMINAL_H
