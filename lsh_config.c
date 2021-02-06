
/**
 *
 */ 

#include <stdio.h>
#include <string.h> 
#include <ctype.h>
#include "lsh_config.h"

int open_config() {
    file = fopen(".lshrc", "r");
    if (file == NULL) {
        end_file = 1;
        return 0;
    }
    end_file = 0;
    return 1;
}

int close_config() {
    fclose(file);
    end_file = 1;
    return 0;
}


// end of file return 0
int read_config(char * line, int n) {
    memset(line, '\0', n);
    line = fgets(line, n, file);
    if (line == NULL) {
        return 0;
    }
    while (isspace(*line))
        line++;
    if (*line) {
        if (*line == '#')
            return -1;
        return sizeof(line);
    }
    return -1;
}

int test() {
    open_config();
    char line[4096];
    while (read_config(line, 4096) > 0) {
        printf("12 %s\n", line);
    }
    return 0;
}

