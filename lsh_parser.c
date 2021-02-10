
/**
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>

#include "lsh_parser.h"

#define GET_STDIO_FILENAME() \
    if (token[len]) { strcpy(file, token + len); } \
    else {\
        if (*idx == 0) {\
            PRI_ERR("lsh syntax error\n");\
            return -1;\
        } \
        split_command(cmd, idx, file);\
    }

#define EXCEPT_FILE_ERROR() \
    if (*fd < 0) {\
        PRI_ERR("Open File %s Error\n", file);\
        return -1;\
    }


int reset_stdio() {

    if (dup2(std_in, fileno(stdin)) < 0) {
        PRI_ERR("RESET IO STDIN Error\n");
        return -1;
    }
    if (dup2(std_out, fileno(stdout)) < 0) {
        PRI_ERR("RESET IO STDOUT Error\n");
        return -1;
    }
    if (dup2(std_err, fileno(stderr)) < 0) {
        PRI_ERR("RESET IO STDERR Error\n");
        return -1;
    }

    return 0;
}

int set_stdio(int in, int out, int err) {
    std_in = dup(fileno(stdin));
    std_out = dup(fileno(stdout));
    std_err = dup(fileno(stderr));

    if (dup2(in, fileno(stdin)) < 0) {
        PRI_ERR("Redirection STDIN Error\n");
        return -1;
    }
    if (dup2(out, fileno(stdout)) < 0) {
        PRI_ERR("Redirection STDOUT Error\n");
        return -1;
    }
    if (dup2(err, fileno(stderr)) < 0) {
        PRI_ERR("Redirection STDERR Error\n");
        return -1;
    }
    return 0;
}

int split_command(const char * cmd, int * idx, char * token) {
    cmd = cmd + *idx;
    while (*cmd && isspace(*cmd)) {
        ++cmd;
        (*idx)++;
    }
    int in_quote = 0, quote = 0, i = 0;
    while (*cmd) {
         if (*cmd == ' ' && in_quote == 0)
            break;
        if (*cmd == '\'' || *cmd == '"') {
            if (in_quote == 0) {
                in_quote = 1;
                quote = *cmd;
            } else if (quote == *cmd) {
                in_quote = 0;
                quote = 0;
            } else {
                token[i++] = *cmd;
            }
        } else {
            token[i++] = *cmd;
        }
        ++cmd, (*idx)++;
    }

    token[i] = '\0';

    if (!(*cmd)) 
        *idx = 0;

    return *idx;
}

int is_start_with(const char * src, const char * start) {
    int match_len = 0;
    while (*src && *start) {
        if (*src != *start)
            return 0;
        ++src, ++start, ++match_len;
    }

    if (*src && !(*start)) 
        return match_len;

    if (!(*src) && *start)
        return 0;

    return match_len;
}

int handle_token(const char * token, char ** argv, int * pos, const char * cmd, int * idx) {
    int len = 0;
    char file[LINEMAX];
    int out_file = STDOUT_FILENO, in_file = STDIN_FILENO, err_file = STDERR_FILENO;
    int out_open = 0, in_open = 0, err_open = 0;
    int * fd;

    if ((len = is_start_with(token, ">>")) || (len = is_start_with(token, "1>>"))) {
        GET_STDIO_FILENAME();
        fd = &out_file;
        *fd = open(file, O_CREAT | O_APPEND | O_RDWR, 0664);
        EXCEPT_FILE_ERROR();
        out_open = 1;
    
    } else if ((len = is_start_with(token, ">")) || (len = is_start_with(token, "1>"))){
        GET_STDIO_FILENAME(); 
        fd = &out_file;
        *fd = open(file, O_CREAT | O_TRUNC | O_RDWR, 0664);
        EXCEPT_FILE_ERROR();
        out_open = 1;

    } else if ((len = is_start_with(token, "<"))){
        GET_STDIO_FILENAME();
        fd = &in_file;
        *fd = open(file, O_RDONLY);
        EXCEPT_FILE_ERROR();
        in_open = 1;

    } else if ((len = is_start_with(token, "2>>"))) {
        GET_STDIO_FILENAME();
        fd = &err_file;
        *fd = open(file, O_CREAT | O_APPEND | O_RDWR, 0664);
        EXCEPT_FILE_ERROR();
        err_open = 1;

    } else if (strcmp(token, "2>&1") == 0) {
        err_file = out_file;
        err_open = 1;

    } else if ((len = is_start_with(token, "2>"))) {
        GET_STDIO_FILENAME();
        fd = &err_file;
        *fd = open(file, O_CREAT | O_TRUNC | O_RDWR, 0664);
        EXCEPT_FILE_ERROR();
        err_open = 1;

    } else {
        argv[*pos] = (char *)calloc(LINEMAX, sizeof(char));
        strcpy(argv[*pos], token);
        (*pos)++;
    }

    if (in_open | out_open | err_open) {
        set_stdio(in_file, out_file, err_file);
    }

    return 0;
}

int command_parser(const char * cmd, char ** argv) {
    char token[LINEMAX];
    int idx = 0, pos = 0;
    do {
        split_command(cmd, &idx, token);
        handle_token(token, argv, &pos, cmd, &idx);
    } while (idx);

    return pos;
}
    


