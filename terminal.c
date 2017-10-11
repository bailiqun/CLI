#include "terminal.h"
#include "cmd_help.h"

void usart_putchar(const char* const ptr)
{
    putc(*ptr, stdout);
}

int register_cmd(char *name, char *usage, cmd_fun_t fun)
{
    int ret;
    if (cmd_num_current < MAX_CMD_NUM)
    {
        strcpy(CmdTbl[cmd_num_current].name, name);
        strcpy(CmdTbl[cmd_num_current].usage,usage);
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


int32_t readline_into_buffer ( char* const prompt, char* buffer)
{
    char *p = buffer;
    char *p_buf = p;
    int n = 0;              /* buffer index     */
    int col;                /* output column cnt    */
    char c;

    char *ptr = prompt;
    int prompt_len = 0;     /* prompt length    */

    /* print prompt */
    if (prompt)
    {
        prompt_len = strlen (prompt);
        while (*ptr)
        {
            usart_putchar(ptr);
            ptr++;
        }
    }

    col = prompt_len;

    while (1)
    {
        c = getc(stdin);
        switch (c)
        {
            case '\r':              /* Enter        */
            case '\n':
                *p = '\0';
                return (p - p_buf);

            case '\0':              /* nul          */
                continue;

            case 0x03:              /* ^C - break       */
                p_buf[0] = '\0';    /* discard input */
                return (-1);

            case 0x15:              /* ^U - erase line  */
                while (col > prompt_len) {
                    puts (erase_seq);
                    --col;
                }
                p = p_buf;
                n = 0;
                continue;

            case 0x17:              /* ^W - erase word  */
                p=delete_char(p_buf, p, &col, &n, prompt_len);
                while ((n > 0) && (*p != ' ')) {
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
                    if (c == '\t')
                    {   /* expand TABs      */
                        col += 8 - (col&07);
                    }
                    else
                    {
                        ++col;      /* echo input       */
                    }
                    *p++ = c;
                    ++n;
                }
                else
                {/* Buffer full      */
                    putc ('\a', stdout);
                }
        }
    }
}


int32_t readline ( char *const prompt)
{
    /*
     * If console_buffer isn't 0-length the user will be prompted to modify
     * it instead of entering it from scratch as desired.
     */
    memset(console_buffer, '\0', CONFIG_SYS_CBSIZE+1);
    console_buffer[0] = '\0';

    return readline_into_buffer(prompt, console_buffer);
}


int find_cmd(char *cmd)
{
    int cmd_index = 0;
    if (('0' == cmd[0]) && ('\0' == cmd[1]))
    return cmd_index;

    cmd_index = 0;
    while (cmd_index < MAX_CMD_NUM)
    {
        if (0 == strncmp(CmdTbl[cmd_index].name, cmd, strlen(cmd)))
        return cmd_index;
        cmd_index++;
    }
    printf("Command  [%s]  don't support!\n", cmd);
    return -1 ;
}

/****************************************************************************
 * returns:
 *  1  - command executed, repeatable
 *  0  - command executed but not repeatable, interrupted commands are
 *       always considered not repeatable
 *  -1 - not executed (unrecognized, bootd recursion or too many args)
 *           (If cmd is NULL or "" or longer than CONFIG_SYS_CBSIZE-1 it is
 *           considered unrecognized)
 *
 * WARNING:
 *
 * We must create a temporary copy of the command since the command we get
 * may be the result from getenv(), which returns a pointer directly to
 * the environment data, which may change magicly when the command we run
 * creates or modifies environment variables (like "bootp" does).
 */
int run_command (const char * const cmd, int flag)
{
    //puts(cmd);

    int cmd_index = 0;
    int argc = 0;
    char *argv[MAX_ARGC];

    argc = parse_line(cmd, argv);

    if ((argc > 0) && (argc < MAX_ARGC))
        cmd_index = find_cmd(argv[0]);
    else
        return 1;


    if (cmd_index != -1)
        CmdTbl[cmd_index].CmdFun(argc, argv);

    return 0;

#if 0
    cmd_tbl_t *cmdtp;
    char cmdbuf[CONFIG_SYS_CBSIZE]; /* working copy of cmd      */
    char *token;            /* start of token in cmdbuf */
    char *sep;          /* end of token (separator) in cmdbuf */
    char finaltoken[CONFIG_SYS_CBSIZE];
    char *str = cmdbuf;
    char *argv[CONFIG_SYS_MAXARGS + 1]; /* NULL terminated  */
    int argc, inquotes;
    int repeatable = 1;
    int rc = 0;

#ifdef DEBUG_PARSER
    printf ("[RUN_COMMAND] cmd[%p]=\"", cmd);
    puts (cmd ? cmd : "NULL");  /* use puts - string may be loooong */
    puts ("\"\n");
#endif

    clear_ctrlc();      /* forget any previous Control C */

    if (!cmd || !*cmd) {
        return -1;  /* empty command */
    }

    if (strlen(cmd) >= CONFIG_SYS_CBSIZE) {
        puts ("## Command too long!\n");
        return -1;
    }

    strcpy (cmdbuf, cmd);

    /* Process separators and check for invalid
     * repeatable commands
     */

#ifdef DEBUG_PARSER
    printf ("[PROCESS_SEPARATORS] %s\n", cmd);
#endif
    while (*str) {

        /*
         * Find separator, or string end
         * Allow simple escape of ';' by writing "\;"
         */
        for (inquotes = 0, sep = str; *sep; sep++) {
            if ((*sep=='\'') &&
                (*(sep-1) != '\\'))
                inquotes=!inquotes;

            if (!inquotes &&
                (*sep == ';') &&    /* separator        */
                ( sep != str) &&    /* past string start    */
                (*(sep-1) != '\\')) /* and NOT escaped  */
                break;
        }

        /*
         * Limit the token to data between separators
         */
        token = str;
        if (*sep) {
            str = sep + 1;  /* start of command for next pass */
            *sep = '\0';
        }
        else
            str = sep;  /* no more commands for next pass */
#ifdef DEBUG_PARSER
        printf ("token: \"%s\"\n", token);
#endif

        /* find macros in this token and replace them */
        process_macros (token, finaltoken);

        /* Extract arguments */
        if ((argc = parse_line (finaltoken, argv)) == 0) {
            rc = -1;    /* no command at all */
            continue;
        }

        /* Look up command in command table */
        if ((cmdtp = find_cmd(argv[0])) == NULL) {
            printf ("Unknown command '%s' - try 'help'\n", argv[0]);
            rc = -1;    /* give up after bad command */
            continue;
        }

        /* found - check max args */
        if (argc > cmdtp->maxargs) {
            cmd_usage(cmdtp);
            rc = -1;
            continue;
        }

#if defined(CONFIG_CMD_BOOTD)
        /* avoid "bootd" recursion */
        if (cmdtp->cmd == do_bootd) {
#ifdef DEBUG_PARSER
            printf ("[%s]\n", finaltoken);
#endif
            if (flag & CMD_FLAG_BOOTD) {
                puts ("'bootd' recursion detected\n");
                rc = -1;
                continue;
            } else {
                flag |= CMD_FLAG_BOOTD;
            }
        }
#endif

        /* OK - call function to do the command */
        if ((cmdtp->cmd) (cmdtp, flag, argc, argv) != 0) {
            rc = -1;
        }

        repeatable &= cmdtp->repeatable;

        /* Did the user stop this? */
        if (had_ctrlc ())
            return -1;  /* if stopped then not repeatable */
    }

    return rc ? rc : repeatable;

#endif
}

void terminal_spin()
{

    int32_t rc = -1;
    int32_t len;
    static char lastcommand[CONFIG_SYS_CBSIZE] = { 0 };

    memset(CmdTbl, 0, sizeof(CMD_STRUCT_T)*MAX_CMD_NUM);
    register_cmd("help", "list all cmd\n\r", HelpCmdExeFun);

    while (1)
    {
        len = readline (CONFIG_SYS_PROMPT);
        if (len > 0)
        {
            memset(lastcommand, '\0', CONFIG_SYS_CBSIZE);
            strncpy (lastcommand, console_buffer, strlen(console_buffer));
            rc = run_command (lastcommand, 0);
            if (rc <= 0)
            {
                /* invalid command or not repeatable, forget it */
                lastcommand[0] = 0;
            }
        }
    }
}

