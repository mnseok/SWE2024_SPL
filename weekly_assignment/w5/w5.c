#include <errno.h>             //errno
#include <linux/limits.h>      //PATH_MAX
#include <stdio.h>             //printf
#include <stdlib.h>            //free
#include <string.h>            //strtok
#include <sys/types.h>         //pid_t
#include <sys/wait.h>          //waitpid
#include <unistd.h>            //fork, execvp

#define MAX_ARG 1000

char* get_line() {
    char *cmd = NULL;
    size_t size = 0;
    getline(&cmd, &size, stdin);
    cmd[strlen(cmd) - 1] = '\0';
    return cmd;
}

char** tokenize(char* cmd) {
    char** args = (char**) malloc(MAX_ARG*sizeof(char*));
    int i = 0;
    char *ptr = strtok(cmd, " "); // Tokenize by space

    while (ptr != NULL) {
        args[i++] = ptr; 
        ptr = strtok(NULL, " "); // Store the token
        // Get the next token
    }

    args[i] = NULL;
    
    return args;
}

void exec(char *args[], char path[]) {
    pid_t pid = fork();
    
    if (pid == -1) {
        fprintf(stderr, "Error: fork failed\n");
        exit(2);
    } else if (pid == 0) {
        if (execv(path, args) == -1) {
            fprintf(stderr, "%s: command not found\n", args[0]);
            exit(1);
        }
    } else {
        int child_status;
        waitpid(pid, &child_status, 0);
    }

    free(args);
}

int main() {
    while(1) {
        char *cmd = get_line();
        char **args = tokenize(cmd);
        char path[100];
        sprintf(path, "/bin/%s", args[0]);
        
        if (strcmp(args[0], "exit") == 0) {
            printf("exit");
            if (args[1] == NULL) {
                exit(0);
            } else {
                exit(atoi(args[1]));
            }
        }
        
        exec(args, path);
        free(cmd);
    }
}
