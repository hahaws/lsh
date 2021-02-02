/**
 *
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>
#include <pwd.h>    
#include <sys/types.h>
#include <sys/wait.h>   

#include "lsh.h"
#include "lsh_dict.h"

int main() {
    map = new_map();
    loop();
    free_map(map);
    return EXIT_SUCCESS;
}

void loop() {
    int i = 0;
    do {
        print_promot();
        if ((i = read_line()) == 1) {
            char ** argv = split_command(cmdLine);
            execute(argv);
            free(argv);
        } 
    } while (1);
}

int exit_lsh() {
    printf("Exit lsh\n");
    exit(0);
}

int get_user() {
    struct passwd * pwd;
    pwd = getpwuid(getuid());
    memset(currUser, '\0', LINEMAX);
    strncpy(currUser, pwd->pw_name, LINEMAX);
    return 0;
}

int get_dir() {
    if (get_user() != 0) {
        PRI_ERR("Get User Name Error\n");
        exit(1);
    }

    char curr_abs_path[LINEMAX];
    char user_home[LINEMAX];
    memset(curr_abs_path, '\0', LINEMAX);
    memset(user_home, '\0', LINEMAX);

    if (NULL == getcwd(curr_abs_path, LINEMAX)) {
        PRI_ERR("Promot Get Dir Path Error\n");
        exit(1);
    }

    int path_len = strlen(curr_abs_path);
    int user_len = strlen(currUser);

    int path_user_idx = path_len - user_len;

    strcat(user_home, "/home/");
    strcat(user_home, currUser);

    int user_home_len = strlen(user_home);
    int is__user = 1;

    int i = 0;
    for (; i < user_home_len; ++i) {
        if (user_home[i] != curr_abs_path[i]) {
            is__user = 0;
            break;
        }
    }

    memset(currPath, '\0', LINEMAX);
    int offset = 0;
    if (is__user) {
        strcat(currPath, "~");
        offset = strlen(user_home);
    }
    strcat(currPath, curr_abs_path + offset);

    return 0;
}

int print_promot() {
    int i = get_dir();
    if (0 == i) {
        printf("lsh %s > ", currPath);
    }
    return i;
}

int sigint() {
    if (cmd_idx == 0 && (getchar() == 3)) {
        printf("\n\r");
        system("stty cooked");
        exit_lsh();
    } else {
        printf("\n\r");
    }
    return 0;
}

int cleanscreen() {
    char * argv[2];
    argv[0] = "clear";
    argv[1] = NULL;
    pid = fork();
    if (pid < 0) {
        PRI_ERR("Fork Error\n");
    } else if (pid == 0) {
        execvp(argv[0], argv);
        printf("Execute clear Error\n");
        exit(127);
    }

    if ((pid = waitpid(pid, &pid_status, 0)) < 0) {
        PRI_ERR("Execute clear Error\n");
        return 1;
    }

    return 0;
}

int carriage() {
    printf("\b\b  \b\b\r\n");
    return 0;
}

int backspace() {
    printf("\b\b  \b\b");
    if (cmd_idx) {
        printf("\b \b");
        cmdLine[--cmd_idx] = '\0';
    }
    return 0;
}

int read_line() {
    memset(cmdLine, '\0', LINEMAX);
    cmd_idx = 0;

    int c = 0, read_state = 0;

    system("/bin/stty raw");

    do {
        switch(c = getchar()) {
            case 3:
                sigint();
                read_state = -1;
                break;
            case 12:
                cleanscreen();
                read_state = -1;
                break;
            case 13:
                carriage();
                read_state = 1;
                break;
            case 127:
                backspace();
                break;
            default:
                cmdLine[cmd_idx++] = c;
        }
    } while (!read_state);

    system("/bin/stty cooked");

    return read_state;
}

char ** split_command(const char * cmd) {

    char * t = strdup(cmd), * tt = strdup(cmd);
    int seplen = 1;
    while (strsep(&t, " ") != NULL) 
        seplen ++;
    char ** argv = (char **)malloc(seplen * sizeof(char *));
    memset(argv, 0, seplen);

    char * tk;
    int idx = 0;
    do {
        tk = strsep(&tt, " ");
        argv[idx++] = tk;
    } while (tk != NULL);

    free(t);
    free(tt);

    return argv;
}

int execute(char ** argv) {
    int i = 0;
    sd_entry * entry;
    for (; i < builtin_func_cnt(); ++i) {
        if (strcmp(argv[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(argv);
        } else if (argv[1] == NULL && (entry = find_entry(map, argv[0])) != NULL) {
            char * cmd = (char *)entry->value;
            char ** _argv = split_command(cmd);
            int res = execute(_argv);
            free(_argv);
            return res;
        }
    }
    return execute_process(argv);
}

int execute_process(char ** argv) {
    pid = fork();
    if (pid < 0) {
        PRI_ERR("Fork Error\n");
        return -1;
    } else if (pid == 0) {
        execvp(argv[0], argv);
        PRI_ERR("Execute %s Error\n", cmdLine);
        exit(127);
    }

    if ((pid = waitpid(pid, &pid_status, 0)) < 0) {
        PRI_ERR("Waitpid Error\n");
        return -1;
    }

    return 0;
}

int builtin_func_cnt() {
    return sizeof(builtin_str) / sizeof(char *);
}

int builtin_alias(char ** argv) {
    if (argv[1] == NULL) {
        PRI_ERR("Nothing alias \n");
        return -1;
    } 

    if (argv[2] == NULL) {
        PRI_ERR("alias to NULL \n");
        return -1;
    }

    if (strcmp(argv[1], argv[2]) == 0) {
        PRI_ERR("Can Not alias to same \n");
        return -1;
    }

    char * alias_to = malloc(LINEMAX * sizeof(char));
    char * alias_from = malloc(LINEMAX * sizeof(char));

    memset(alias_to, '\0', LINEMAX * sizeof(char));
    memset(alias_from, '\0', LINEMAX * sizeof(char));

    strcat(alias_to, argv[1]);

    int idx = 2;
    for (;;) {
        strcat(alias_from, argv[idx++]);
        if (argv[idx] == NULL) 
            break;
        strcat(alias_from, " ");
    }

    insert_map(map, alias_to, alias_from);

    return 0;
}

int builtin_unalias(char ** argv) {
    if (argv[1] == NULL) {
        PRI_ERR("Nothing to unalias\n");
        return -1;
    }
    int idx = idx_entry(map, argv[1]);
    if (idx != 0) {
        delete_entry_idx(map, idx);
    }
    printf("unalias %s\n", argv[1]);
    return 0;
}

int builtin_cd(char ** argv) {
    
    char * to_path = argv[1];

    if (to_path == NULL) {
        to_path = "~";
    }

    char to_abs_path[LINEMAX];
    memset(to_abs_path, '\0', LINEMAX);

    if (to_path[0] == '~') {
        strcat(to_abs_path, "/home/");
        strcat(to_abs_path, currUser);
        strcat(to_abs_path, to_path + 1);
    } else {
        strcat(to_abs_path, to_path);
    }

    if (chdir(to_abs_path) != 0) {
        PRI_ERR("chdir %s Error\n", to_path);
        return -1;
    }

    return 0;
}
int builtin_exit(char ** argv) {
    exit_lsh();
    return 0;
}
int builtin_help(char ** argv) {
    printf("running help\n");
    return 0;
}
