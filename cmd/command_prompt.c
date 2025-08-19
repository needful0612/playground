#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <stdbool.h>
#include <ctype.h>
#include <termios.h>
#include <signal.h>
#include <errno.h>

#define HISTORY_SIZE 100
#define BUFFER_SIZE 4096

char *history[HISTORY_SIZE];
int history_count = 0;
int history_index = -1;

typedef void (*CommandFunc)(int argc, char** argv);
static char prev_dir[PATH_MAX] = "";

int copy_file(const char *src_path, const char *dest_path);

typedef struct {
    const char* name;
    CommandFunc func;
    int min_args;
    int max_args; 
} Command;

void cleanup() {
    for (int i = 0; i < history_count; i++) {
        free(history[i]);
    }
    exit(0);
}

int copy_file(const char *src_path,const char *dest_path) {
    FILE *src = fopen(src_path, "rb");
    if (!src) {
        fprintf(stderr, "cannot open source file at %s: %s\n",src_path ,strerror(errno));
        return -1;
    }

    FILE *dest = fopen(dest_path, "wb");
    if (!dest) {
        fprintf(stderr, "cannot open destination file at %s: %s\n",dest_path ,strerror(errno));
        fclose(src);
        return -1;
    }

    char buffer[BUFFER_SIZE];
    size_t bytes_read, bytes_written;

    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, src)) > 0) {
        bytes_written = fwrite(buffer, 1, bytes_read, dest);
        if (bytes_written < bytes_read) {
            fprintf(stderr, "error writing file at %s: %s\n", dest_path, strerror(errno));
            fclose(src);
            fclose(dest);
            return -1;
        }
    }

    if (ferror(src)) {
        fprintf(stderr, "error reading file at %s: %s\n", src_path, strerror(errno));
        fclose(src);
        fclose(dest);
        return -1;
    }

    fclose(src);
    fclose(dest);
    return 0;
}

void add_to_history(const char* command) {
    if(history_count < HISTORY_SIZE) {
        history[history_count] = strdup(command);
        history_count++;
    } else {
        free(history[0]);
        for(int i = 1;i < HISTORY_SIZE;i++) {
            history[i - 1] = history[i];
        }
        history[HISTORY_SIZE - 1] = strdup(command);
    }
}

void handle_signal(int sig) {
    //printf("\nReceived signal %d\n", sig);
    cleanup();
    exit(0);
}

void handle_exit(int argc, char** argv) {
    cleanup();
    exit(0);
}

void handle_unknown() {
    printf("unknown command\n");
}

void handle_pwd(int argc, char** argv) {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("getcwd");
        return;
    }
}

void handle_history(int argc, char** argv) {
    for (int i = 0; i < history_count; i++) {
        printf("%d %s\n", i + 1, history[i]);
    }
}

void handle_cd(int argc, char** argv) {
    char* path;
    char cwd[PATH_MAX];

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        return;
    }

    if (argc == 0 || argv[0] == NULL || strcmp(argv[0], "~") == 0) {
        path = getenv("HOME");
        if (path == NULL) {
            printf("cd: HOME environment variable not set\n");
            return;
        }
    } else if (strcmp(argv[0], "-") == 0) {
        if (prev_dir[0] == '\0') {
            return;
        }
        path = prev_dir;
        printf("%s\n", path);
    } else {
        path = argv[0];
    }

    if (chdir(path) != 0) {
        perror("cd");
    } else {
        strcpy(prev_dir, cwd);
    }
}

void handle_ls(int argc, char** argv) {
    DIR* dir = opendir(".");

    if (dir == NULL) {
        perror("opendir");
        return;
    }

    struct dirent* entry;

    while((entry = readdir(dir)) != NULL){
        if(entry->d_name[0] != '.'){
            printf("%s ",entry->d_name);
        }
    }

    printf("\n");
    closedir(dir);
}

void handle_cp(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: cp source_file destination_file\n");
        return;
    }

    copy_file(argv[0], argv[1]);
}

void handle_mv(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: mv source_file destination_file\n");
        return;
    }

    const char *src = argv[0];
    const char *dest = argv[1];

    if (rename(src, dest) != 0) {
        if (copy_file(src, dest) != 0) {
            return;
        }

        if (unlink(src) != 0) {
            fprintf(stderr, "failed to remove source file %s: %s\n", src, strerror(errno));
        }
    }
}

void handle_echo(int argc, char** argv) {
    for (int i = 0; i < argc; i++) {
        printf("%s", argv[i]);
        if (i < argc - 1) {
            printf(" ");
        }
    }
    printf("\n");
}

Command command_table[] = {
    {"exit", handle_exit, 0, 0},
    {"pwd", handle_pwd, 0, 0},
    {"history", handle_history, 0, 0},
    {"cd", handle_cd, 0, 1},
    {"ls", handle_ls, 0, 1},
    {"cp", handle_cp, 2, 2},
    {"mv", handle_mv, 2, 2},
    {"echo", handle_echo, 0, -1},
};

void free_args(char** args, int size) {
    for (int i = 0; i < size; i++) {
        free(args[i]);
    }
    free(args);
}

char** parse_args(char* command, int* arg_count) {
    int capacity = 2;
    int size = 0;
    char** args = malloc(sizeof(char*) * capacity);
    if (!args) return NULL;

    if(!command) return NULL;
    const char* p = command;

    while (*p) {
        while (isspace(*p)) p++;
        if (*p == '\0') break;

        char* arg = malloc(PATH_MAX);
        if (!arg) {
            perror("malloc");
            free_args(args, size);
            return NULL;
        }

        int i = 0;
        char quote = 0;

        if (*p == '\'' || *p == '"') {
            quote = *p++;
        }

        while (*p && ((quote && *p != quote) || (!quote && !isspace(*p)))) {
            if (*p == '\'' || *p == '"') {
                p++;
                if (*p) arg[i++] = *p++;
            } else {
                arg[i++] = *p++;
            }
        }

        if (quote && *p == quote) p++;

        arg[i] = '\0';

        if (size == capacity) {
            capacity *= 2;
            char** temp = realloc(args, sizeof(char*) * capacity);
            if (!temp) {
                printf("realloc failed");
                free(arg);
                free_args(args, size);
                return NULL;
            }
            args = temp;
        }

        args[size++] = strdup(arg);
        free(arg);
    }

    *arg_count = size;
    return args;
}

void parse(char* command) {
    char* tmp = command;
    char* token = strtok(tmp, " ");
    if (token == NULL) {
        return;
    }

    char* remainded_args = strtok(NULL, "");

    int argc = 0;
    char** args = parse_args(remainded_args, &argc);
    if (!args && argc > 0) {
        printf("parse_args failed");
        return;
    }

    size_t command_table_size = sizeof(command_table) / sizeof(command_table[0]);
    bool found = false;

    for (int i = 0; i < command_table_size; i++) {
        if (strcmp(token, command_table[i].name) == 0) {
            found = true;

            if (argc < command_table[i].min_args ||
                (command_table[i].max_args != -1 && argc > command_table[i].max_args)) {
                printf("%s expects %d to %d argument(s), got %d\n",
                    command_table[i].name,
                    command_table[i].min_args,
                    command_table[i].max_args == -1 ? 999 : command_table[i].max_args,
                    argc
                );
                break;
            }

            command_table[i].func(argc, args);
            break;
        }
    }

    if (!found) {
        handle_unknown();
    }

    free_args(args, argc);
}

int main() {
    char input[100];
    char cwd[PATH_MAX];

    signal(SIGINT, handle_signal);   // Ctrl+C
    signal(SIGTERM, handle_signal);  // Termination
    signal(SIGQUIT, handle_signal);  // Ctrl+\ 
    signal(SIGTSTP, handle_signal);  // Ctrl+Z
    signal(SIGABRT, handle_signal);  // Abort

    while (1) {
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s$ ", cwd);
        } else {
            perror("getcwd() error");
            return 1;
        }

        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) > 0) {
            add_to_history(input);
            parse(input);
        }
    }

    for (int i = 0; i < history_count; i++) {
        free(history[i]);
    }

    return 0;
}
