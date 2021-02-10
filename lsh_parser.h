
/**
 *
 */ 

#ifndef __LSH_PARSER_H__
#define __LSH_PARSER_H__

#include <stdio.h>

#define LINEMAX 4096
#define PRI_ERR printf


static int std_out, std_in, std_err;

int is_start_with(const char * src, const char * start);
int command_parser(const char * cmd, char ** argv);
int handle_token(const char * token, char ** argv, int * pos, const char * cmd, int * idx);
int split_command(const char * cmd, int * idx, char * token);
int set_stdio(int in, int out, int err);
int reset_stdio();

#endif // __LSH_PARSER_H__
