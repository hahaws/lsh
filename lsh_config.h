
/**
 *
 */ 


#ifndef __LSH_CONFIG_H__
#define __LSH_CONFIG_H__

#include <stdio.h>

FILE * file;
int end_file;

int open_config();
int read_config(char * line, int n);
int close_config();
int config_end();

#endif // __LSH_CONFIG_H__
