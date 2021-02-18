/**
 *
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>    
#include <ctype.h>
#include <fcntl.h>
#include <time.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>   

#include "lsh.h"
#include "lsh_config.h"
#include "lsh_dict.h"
#include "lsh_parser.h"

int main(int argc, char ** argv) {
    if (argc == 1) {
        init();
        loop();
        free_map(&map);
    } else {
        handle_argv(argc, argv);
    }
    return EXIT_SUCCESS;
}

void handle_argv(int argc, char ** argv) {
    if (argc == 2) {
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            builtin_help(NULL);
            return;
        }
    } 

    printf("lsh: Unrecongnized option '%s'\nTry: 'lsh -h' or 'lsh --help' for more information\n", argv[1]);
}

void init() {
    tcgetattr(STDIN_FILENO, &oldt);
    signal(SIGINT, SIG_IGN);
    get_path();
    map = new_map();
    open_config();
    char line[LINEMAX];
    int can = 0;
    while ((can = read_config(line, LINEMAX)) != 0) {
        if (can > 0) {
            run_command(line);
        }
    }
    close_config();
}

void loop() {
    do {
        print_promot();
        if (read_line() == 1) {
            run_command(runCmd);
        }
    } while (1);
}

int exit_lsh() {
    free_map(&map);
    printf("Exit lsh\n");
    exit(0);
}

int is_path(const char * cmd) {
    while (*cmd) {
        if (*cmd == '/') {
            return 1;
        }
        cmd ++;
    }
    return 0;
}

int cmd_access(const char * cmd, char * abs_path, int len) {
    int i = 0, x = -1;
    for (; i < PATH_CNT; ++i) {
        memset(abs_path, '\0', len);
        strcat(abs_path, PATH[i]);
        strcat(abs_path, cmd);
        if (access(abs_path, F_OK) < 0) {
            continue;
        }
        if (access(abs_path, X_OK) == 0) {
            x = 0;
            break;
        }
    }
    return x;
}

int get_time() {
    time_t local_time;
    struct tm * t;
    time(&local_time);
    t = gmtime(&local_time);
    memset(CURRTIME, '\0', LINEMAX);
    sprintf(CURRTIME, "%02d:%02d:%02d", t->tm_hour + 8, t->tm_min, t->tm_sec);
    return 0;
}

int get_path() {
    char * all_path;
    all_path = getenv("PATH");
    int i = 0, j = 0, idx = 0;
    while (all_path[idx]) {
        if (all_path[idx] == ':') {
            if (PATH[i][j] != '/') 
                PATH[i][j++] = '/';
            PATH[i][j] = '\0';
            ++i;
            j = 0;
        } else {
            PATH[i][j++] = all_path[idx];
        }
        ++idx;
    }
    if (PATH[i][j] != '/') 
        PATH[i][j++] = '/';
    PATH[i][j] = '\0';

    PATH_CNT = i;
    return i;
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

    memset(CURPWD, '\0', LINEMAX);
    int offset = 0;
    if (is__user) {
        strcat(CURPWD, "~");
        offset = strlen(user_home);
    }
    strcat(CURPWD, curr_abs_path + offset);

    return 0;
}

int print_promot() {
    int i = get_dir();
    get_time();
    if (0 == i) {
        printf("%s %s > %s", CURRTIME, CURPWD, cmdLine);
    }
    return i;
}

int keydonw_ctrl_c() {
    printf("^C");
    if (cmd_idx == 0) {
        printf("\nPress Ctrl-C again to quit lsh");
        if (3 == getchar()) {
            printf("\n\r");
            tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
            exit_lsh();
        } else {
            memset(cmdLine, '\0', LINEMAX);
            cmd_idx = 0;
            printf("\n\r");
        }
    } else {
        memset(cmdLine, '\0', LINEMAX);
        cmd_idx = 0;
        printf("\n\r");
    }

    return 0;
}

int keydown_ctrl_l() {
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

int keydown_enter() {
    strcpy(runCmd, cmdLine);
    memset(cmdLine, '\0', LINEMAX);
    cmd_idx = 0;
    printf("\r\n");
    return 0;
}

int keydown_backspace() {
    if (cmd_idx) {
        printf("\b \b");
        cmdLine[--cmd_idx] = '\0';
    }
    return 0;
}

int keydown_tab() {
    int len = strlen(cmdLine);
    if (len <= 0) {
        return -1;
    }
    char tdir[LINEMAX] = {0};
    char pre[LINEMAX] = {0};
    int pre_len = 0;
    if (isspace(cmdLine[len - 1]))
        strcpy(tdir, "./");
    else {
        int i = len - 1;
        while (i >= 0 && !isspace(cmdLine[i]))
            --i;
        int j = i, ii = i + 1;
        while (ii < len) {
            if (cmdLine[ii] == '/')
                j = ii;
            ++ii;
        }
        strncpy(tdir, cmdLine + i, j - i + 1);
        strcpy(pre, cmdLine + j + 1);
    } 
    pre_len = strlen(pre);

    strip(tdir);
    strip(pre);
    
    if (!*tdir) {
        strcat(tdir, "./");
    }

    char pdir[LINEMAX] = {0};
    if (tdir[0] == '~') {
        strcat(pdir, "/home/");
        strcat(pdir, currUser);
        strcat(pdir, tdir + 1);
    } else {
        strcat(pdir, tdir);
    }

    
    DIR * _dir;
    struct dirent * ptr;
    _dir = opendir(pdir);
    if (_dir == NULL) {
        return 0;
    }
    char ** files = (char **)calloc(LINEMAX, sizeof(char *));
    int files_cnt = 0;
    int l = 0, dif = 0;
    while ((ptr = readdir(_dir)) != NULL) {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)
            continue;
        if (pre_len == 0 || (l = is_start_with(ptr->d_name, pre))) {
            files[files_cnt] = (char *)calloc(LINEMAX, sizeof(char));
            char pth[LINEMAX] = {0};
            strcat(pth, pdir);
            strcat(pth, ptr->d_name);
            struct stat st;
            stat(pth, &st);
            if (S_ISDIR(st.st_mode))
                strcat(ptr->d_name, "/");
            strcat(files[files_cnt++], ptr->d_name);
            dif = l;
        }
    }
    if (files_cnt == 0) {
        free(files);
        closedir(_dir);
        return 0;
    }
    else if (files_cnt == 1) {
        char * p = files[0];
        char * f = p + dif;
        while (*f)
            show_char(*f++);
        show_char(' ');
        free(p);
        free(files);
        closedir(_dir);
        return 0;
    }
    else if (files_cnt > 1) {
        printf("\n");
        do {
            printf("%s\t", files[--files_cnt]);
            free(files[files_cnt]);
        } while (files_cnt);
        printf("\n");
        free(files);
        closedir(_dir);
        return -1;
    }
    return -1;
}

int show_char(int c) {
    if (isprint(c)) {
        cmdLine[cmd_idx++] = (char)c;
        printf("%c", c);
    }
    return 0;
}

int read_line() {

    int c = 0, read_state = 0;

    tcgetattr(STDIN_FILENO, &oldt);

    newt = oldt;
    newt.c_lflag &= ~ECHO;
    newt.c_lflag &= ~ICANON;
    newt.c_lflag &= ~ISIG;

    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    do {
        c = getchar();
        switch(c) {
            case 3:     // CTRL-C
                keydonw_ctrl_c();
                read_state = -1;
                break;
            case 12:    // CTRL-L
                keydown_ctrl_l();
                read_state = -1;
                break;
            case 9:     // tab
                read_state = keydown_tab();
                break;
            case 10:
                keydown_enter();
                read_state = 1;
                break;
            case 13:
                keydown_enter();
                read_state = 1;
                break;
            case 127:
                keydown_backspace();
                break;
            default:
                show_char(c);
                break;
        }
    } while (!read_state);
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return read_state;
}

int strip(char * cmd) {
    size_t size;
    char * end;
    
    size = strlen(cmd);

    if (!size) {
        return 0;
    }

    end = cmd + size - 1;
    while (end >= cmd && isspace(*end))
        end--;
    *(end + 1) = '\0';

    char * s = cmd;
    while (*s && isspace(*s))
        s++;

    while (*s) {
        *cmd = *s;
        cmd++;
        s++;
    }
    *cmd = '\0';

    return 0;
}

char ** lines_calloc() {
    char ** lines = (char **)calloc(LINEMAX, sizeof(char *));
    int i = 0;
    for (; i < LINEMAX; ++i) 
        lines[i] = NULL;
    return lines;
}

void free_lines(char *** plines) {
    if (*plines == NULL) {
        return;
    }
    char ** lines = *plines;
    int i = 0;
    for (; i < LINEMAX; ++i) {
        if (lines[i]) {
            free(lines[i]);
            lines[i] = NULL;
        }
    }
    free(lines);
    *plines = NULL;
}


int run_command(char * cmd) {
    strip(cmd);
    if (!(*cmd) && cmd[0] == '\0')
        return 0;
    char ** argv = lines_calloc();
    int res = 0;
    int n = 0;
    if ((n = command_parser(cmd, argv))) {
        res = execute(argv);
    }
    reset_stdio();
    free_lines(&argv);
    return res;
}

int execute(char ** argv) {
    int i = 0;
    sd_entry * entry;
    for (; i < builtin_func_cnt(); ++i) {
        if (strcmp(argv[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(argv);
        }
    }

    if (argv[1] == NULL && (entry = find_entry(map, argv[0])) != NULL) {
        char * cmd = (char *)entry->value;
        return run_command(cmd);
    }

    char abs_path[LINEMAX];
    if (is_path(argv[0]) == 0) {
        int ret = cmd_access(argv[0], abs_path, LINEMAX);
        if (ret == 0) {
            memset(argv[0], '\0', LINEMAX);
            strcpy(argv[0], abs_path);
        } else {
            PRI_ERR("Command '%s' not found.\n", argv[0]);
            return 0;
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
        execv(argv[0], argv);
        PRI_ERR("Execute Program `%s` Error\n", argv[0]);
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

    strcat(alias_from, argv[2]);

    int idx = 3;
    while (argv[idx]) {
        strcat(alias_from, " ");
        strcat(alias_from, argv[idx]);
        ++idx;
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

    if (to_path[0] == '-') {
        if (OLDPWD_SET == 0) {
            PRI_ERR("lsh: cd OLDPWD Not set\n");
            return -1;
        }
        to_path = OLDPWD;
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

    struct stat info;
    stat(to_abs_path, &info);
    if (!S_ISDIR(info.st_mode)) {
        PRI_ERR("lsh: cd %s is not a directory\n", to_path);
        return -1;
    }

    if (chdir(to_abs_path) != 0) {
        PRI_ERR("chdir %s Error\n", to_path);
        return -1;
    }

    memset(OLDPWD, '\0', sizeof(OLDPWD));
    strcpy(OLDPWD, CURPWD);
    OLDPWD_SET = 1;

    return 0;
}
int builtin_exit(char ** argv) {
    exit_lsh();
    return 0;
}

int builtin_help(char ** argv) {
    char buf[LINEMAX];
    FILE * file;
    size_t nread;

    file = fopen("./help", "r");
    if (file) {
        while ((nread = fread(buf, 1, sizeof(buf), file)) > 0)
            fwrite(buf, 1, nread, stdout);
        if (ferror(file))
            printf("Read help doc error");
    }
    fclose(file);
    return 0;
}

int builtin_type(char ** argv) {
    int i = 0;
    if (argv[1] == NULL) {
        return 0;
    }
    for (; i < builtin_func_cnt(); ++i) {
        if (strcmp(builtin_str[i], argv[1]) == 0) {
            printf("%s is a lsh builtin function\n", argv[1]);
            break;
        }
    }

    sd_entry * entry = find_entry(map, argv[1]);
    if (entry != NULL) {
        char * cmd = (char *)entry->value;
        printf("%s is aliased to `%s`\n", argv[1], cmd);
    }

    return 0;
}
