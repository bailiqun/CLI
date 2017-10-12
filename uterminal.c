#include "uterminal.h"

/*
 * CMD_NCB_ST command node control block
 * buffer and len is the usart recieve
*/

#ifdef __cplusplus
extern "C" {
#endif
int cmd_find(CMD_NCB_ST *fs);
int cmd_parse_line(CMD_NCB_ST *fs);
int cmd_print_prompt(CMD_NCB_ST *fs);
int cmd_find_similar(CMD_NCB_ST *fs);
int cmd_run(CMD_NCB_ST *fs);
int cmd_find(CMD_NCB_ST *fs);
int cmd_create(CMD_NCB_ST **pfs,printf_st printf_,putc_st putc_,const char * const prompt);
int cmd_register(CMD_NCB_ST *fs, char *name, char *usage, cmd_fun_t fun);
int cmd_destory(CMD_NCB_ST *fs);
int cmd_get_one_cmd_from_buff(CMD_NCB_ST *fs, unsigned char data);
int cmd_read_lines(CMD_NCB_ST *fs, unsigned char data);
//static char * delete_char (char *buffer, char *p, int *colp, int *np, int plen)
//{
//    char *s;
//    if (*np == 0)
//    {
//        return (p);
//    }
//    if (*(--p) == '\t') {           /* will retype the whole line   */
//        while (*colp > plen) {
//            puts (erase_seq);
//            (*colp)--;
//        }
//        for (s=buffer; s<p; ++s) {
//            if (*s == '\t') {
//                puts (tab_seq+((*colp) & 07));
//                *colp += 8 - ((*colp) & 07);
//            } else {
//                ++(*colp);
//                putc (*s, stdout );
//            }
//        }
//    } else {
//        puts (erase_seq);
//        (*colp)--;
//    }
//    (*np)--;
//    return (p);
//}
int cmd_parse_line(CMD_NCB_ST *fs)
{
    int argc = 0;
    const char* const line = fs->rx_buffer;
    char    **argv = fs->argv;
    char *ptr = fs->argv_buffer;
    memset(ptr, '\0', CMD_CONFIG_SYS_CBSIZE);
    strncpy(ptr, line, strlen(line));
    while ((argv[argc]=strtok(ptr, " "))!=NULL)
    {
        argc++;
        if (argc > CMD_MAX_ARGC)
            break;
        ptr = NULL;
    }
    return argc;
}
int cmd_print_prompt(CMD_NCB_ST *fs)
{
    if((fs == NULL)||(fs->is_initialize == FALSE))
        return -1;
    fs->printf("%s",fs->prompt_buffer);
    return 1;
}
int cmd_find_similar(CMD_NCB_ST *fs)
{
    char *cmd = fs->rx_buffer;
    int cmd_index = 0;
    int similar_cmd_count = 0;
    int similar_cmd_item[CMD_MAX_CMD_NUM]={0};
    if((fs == NULL)||(fs->is_initialize == FALSE))
        return -1;
    while (cmd_index < CMD_MAX_CMD_NUM)
    {
        if(fs->cmd_tbl[cmd_index].name == NULL)
            break;
        if (strncmp(fs->cmd_tbl[cmd_index].name, cmd, strlen(cmd)) == 0)
        {
            if(similar_cmd_count > CMD_MAX_CMD_NUM)
                break;
            similar_cmd_item[similar_cmd_count] = cmd_index;
            ++similar_cmd_count;
        }
        cmd_index++;
    }
    fs->printf("\r\n");
    for(int i=0;i<similar_cmd_count;i++)
    {
       fs->printf("[%d]%s\r\n",i,fs->cmd_tbl[similar_cmd_item[i]].name);
    }
    return 1;
}
int cmd_run(CMD_NCB_ST *fs)
{
    if((fs == NULL)||(fs->is_initialize == FALSE))
        return -1;
    int cmd_index = 0;
    fs->argc = cmd_parse_line(fs);
    if ((fs->argc > 0) && (fs->argc < CMD_MAX_ARGC))
        cmd_index = cmd_find(fs);
    else
        return 0;
    if (cmd_index != -1)
    {
        if(fs->cmd_tbl[cmd_index].cmd_cb != NULL)
            fs->cmd_tbl[cmd_index].cmd_cb(fs->argc, fs->argv);
        return 1;
    }
    return 1;
}
int cmd_find(CMD_NCB_ST *fs)
{
    if((fs == NULL)||(fs->is_initialize == FALSE))
        return -1;
    char * cmd = fs->argv[0];
    CMD_STRUCT_T *cmd_tbl = fs->cmd_tbl;
    printf_st printf = fs->printf;
    int cmd_index = 0;
    int similar_cmd_count = 0;
    int similar_cmd_item[CMD_MAX_CMD_NUM]={0};
    while (cmd_index < CMD_MAX_CMD_NUM)
    {
        if(cmd_tbl[cmd_index].name == NULL)  break;
        if (strncmp(cmd_tbl[cmd_index].name, cmd, strlen(cmd)) == 0)
        {
            if(strcmp(cmd,cmd_tbl[cmd_index].name)==0)
            {
                similar_cmd_count =1;
                similar_cmd_item[0] = cmd_index;
                break;
            }
            if(similar_cmd_count > CMD_MAX_CMD_NUM) break;
            similar_cmd_item[similar_cmd_count] = cmd_index;
            ++similar_cmd_count;
        }
        cmd_index++;
    }
    switch(similar_cmd_count)
    {
        case 0:
            printf("Command  [%s ]  don't support!\r\n", cmd);
            return -1;
        case 1:
            return similar_cmd_item[0];
        default:
					  fs->printf("\r\n");
            for(int i=0;i<similar_cmd_count;i++)
                printf("[%d]%s\r\n",i,cmd_tbl[similar_cmd_item[i]].name);
    }
    return -1;
}
int cmd_create(CMD_NCB_ST **pfs,printf_st printf_,putc_st putc_,const char * const prompt)
{
    CMD_NCB_ST *fs;
    fs = ( CMD_NCB_ST *)malloc(sizeof( CMD_NCB_ST));
    fs->printf =printf_;
    fs->putc = putc_;
    fs->cmd_num_current = 0;
    memset(fs->cmd_tbl, '\0', CMD_MAX_CMD_NUM * sizeof(CMD_STRUCT_T));
    memset(fs->rx_buffer, '\0', CMD_CONFIG_SYS_CBSIZE);
     fs->rx_buffer_cnt = 0;
    fs->rx_buffer_size = CMD_CONFIG_SYS_CBSIZE;
    memset(fs->argv_buffer, '\0', CMD_CONFIG_SYS_CBSIZE);
    fs->argc = 0;
    fs->prompt_buffer = prompt;
    fs->prompt_size = strlen(prompt);
    if(fs != NULL)
        fs->is_initialize = TRUE;
    *pfs = fs;
    return 1;
}
int cmd_register(CMD_NCB_ST *fs, char *name, char *usage, cmd_fun_t fun)
{
    if((fs == NULL)||(fs->is_initialize == FALSE)||(name == ""))
        return -1;
    if (fs->cmd_num_current < CMD_MAX_CMD_NUM)
    {
        fs->cmd_tbl[fs->cmd_num_current].name = name;
        fs->cmd_tbl[fs->cmd_num_current].usage = usage;
        fs->cmd_tbl[fs->cmd_num_current].cmd_cb = fun;
        fs->cmd_num_current++;
        return 1;
    }
    else
        return -1;
}
int cmd_destory(CMD_NCB_ST *fs)
{
    free(fs);
    fs = 0;
    return 1;
}
int cmd_get_one_cmd_from_buff(CMD_NCB_ST *fs, unsigned char data)
{
    char *rx_p = fs->rx_buffer;
    unsigned int *rx_cnt =&(fs->rx_buffer_cnt);
    unsigned int rx_size = fs->rx_buffer_size;
    unsigned int ch = data;
    switch(ch)
    {
        case '\r':/* Enter*/
        case '\n':
            rx_p[(*rx_cnt)] = '\0';
            if((*rx_cnt) != 0)
                cmd_run(fs);
						else
							fs->printf("\r\n");
						
            fs->argc = 0;
            fs->rx_buffer_cnt = 0;
            fs->rx_buffer[0] = '\0';
            return 1;
        case '\t':              /*tab*/
						rx_p[(*rx_cnt)] = '\0';			
						if((*rx_cnt) != 0)
								cmd_find_similar(fs);
						else
							fs->printf("\r\n");
						fs->argc = 0;
            fs->rx_buffer_cnt = 0;
            fs->rx_buffer[0] = '\0';
            return 1;
        case '\0':
            return 0;
        default:
            if (*rx_cnt < rx_size - 2)
            {
                rx_p[(*rx_cnt)] = ch;
                (*rx_cnt)++;
            }
            else/*   Buffer full   */
            {
                fs->putc('\a');
            }
            return 0;
    }
//    switch (ch)
//    {
//        case '\r':              /* Enter*/
//        case '\n':
//            p[fs->console_buffer_cnt] = '\0';
//            return fs->console_buffer_cnt;
//        case '\t':
//            find_similar_cmd(fs,p);
//            return 0;
////        case '\0':              /* null          */
////             return 0;;
////        case 0x03:              /* ^C - break       */
////            p_buf[0] = '\0';    /* discard input */
////              return 0;
////        case 0x15:              /* ^U - erase line  */
////            while (col > prompt_len)
////            {
////                puts (erase_seq);
////                --col;
////            }
////            p = p_buf;
////            n = 0;
////             return 0;
////        case 0x17:              /* ^W - erase word  */
////            p=delete_char(p_buf, p, &col, &n, prompt_len);
////            while ((n > 0) && (*p != ' '))
////            {
////                p=delete_char(p_buf, p, &col, &n, prompt_len);
////            }
////              return 0;
////        case 0x08:              /* ^H  - backspace  */
////        case 0x7F:              /* DEL - backspace  */
////            p=delete_char(p_buf, p, &col, &n, prompt_len);
////             return 0;
//        default:
//            /*
//             * Must be a normal character then
//             */
//            if (n < CMD_CONFIG_SYS_CBSIZE-2)
//            {
//                p[fs->console_buffer_cnt] = ch;
//                fs->console_buffer_cnt++;
//            }
//            else/*   Buffer full   */
//            {
//                usart_putchar ("\a");
//            }
//              return 0;
//    }
}
int cmd_read_lines(CMD_NCB_ST *fs, unsigned char data)
{
    if(cmd_get_one_cmd_from_buff (fs,data))
        cmd_print_prompt(fs);
    return 1;
}
#ifdef __cplusplus
}
#endif
