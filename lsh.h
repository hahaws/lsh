
/**
 *
 *
 */ 

#ifndef __LSH_H__
#define __LSH_H__

#include <stdio.h>
#include <sys/types.h>

#include "lsh_dict.h"

#define PRI_ERR printf

#define LINEMAX 4096

const char * builtin_str[] = {
    "alias",
    "cd",
    "exit",
    "help",
    "unalias",
};

int builtin_alias(char ** argv);
int builtin_cd(char ** argv);
int builtin_exit(char ** argv);
int builtin_help(char ** argv);
int builtin_unalias(char ** argv);

int (*builtin_func[]) (char **) = {
    &builtin_alias,
    &builtin_cd,
    &builtin_exit,
    &builtin_help,
    &builtin_unalias,
};

int builtin_func_cnt();

char cmdLine[LINEMAX];
int cmd_idx;

char promot[LINEMAX];
char currUser[LINEMAX];
char currPath[LINEMAX];

pid_t pid;
int pid_status;
sd_map * map;

int exit_lsh();
int sigint();
int carriage();
int backspace();
int cleanscreen();
int get_dir();
int get_user();
int print_promot();
int execute(char ** argv);
int execute_process(char ** argv);
int read_line();
char ** split_command(const char * cmd);
void loop();

#endif // __LSH_H__
