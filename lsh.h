
/**
 *
 *
 */ 

#ifndef __LSH_H__
#define __LSH_H__

#include <stdio.h>
#include <sys/types.h>
#include <termio.h>

#include "lsh_dict.h"

#define PRI_ERR printf

#define LINEMAX 4096

char PATH[LINEMAX][LINEMAX];
int PATH_CNT = 0;

struct termios oldt, newt;

const char * builtin_str[] = {
    "alias",
    "cd",
    "exit",
    "help",
    "type",
    "unalias",
};

int builtin_alias(char ** argv);
int builtin_cd(char ** argv);
int builtin_exit(char ** argv);
int builtin_help(char ** argv);
int builtin_type(char ** argv);
int builtin_unalias(char ** argv);

int (*builtin_func[]) (char **) = {
    &builtin_alias,
    &builtin_cd,
    &builtin_exit,
    &builtin_help,
    &builtin_type,
    &builtin_unalias,
};

int builtin_func_cnt();

char cmdLine[LINEMAX];
int cmd_idx;

char promot[LINEMAX];
char currUser[LINEMAX];
char CURPWD[LINEMAX];
char OLDPWD[LINEMAX];
int OLDPWD_SET = 0;

pid_t pid;
int pid_status;
sd_map * map;

int exit_lsh();
int keydown_enter();
int keydown_backspace();
int keydown_tab();
int keydown_ctrl_l();
int keydonw_ctrl_c();

int show_char(int c);
int get_path();
int cmd_access(const char * cmd, char * abs_path, int len);
int get_dir();
int get_user();
int print_promot();
int execute(char ** argv);
int execute_process(char ** argv);
int read_line();
int strip(char * cmd);
char ** lines_calloc();
void free_lines(char *** lines);
int split_command(const char * cmd, char ** argv);
void loop();

#endif // __LSH_H__
