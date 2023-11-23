#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include "smsh.h"
#include "varlib.h"

int is_control_command(char *);
int do_control_command(char **);
int ok_to_excute();
int builtin_command(char **, int*);

int excute_command(char **args, int background) {
    extern char **environ;
    int pid = fork();
    int child_info = -1;

    if (pid == 0) {
        // Child process
        environ = VLtable2environ();
        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);

        if (execvp(args[0], args) == -1) {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    } 
    else if (pid > 0) {
        // Parent process
        if (!background) {
            // If not running in the background, wait for the child process
            int status;
            waitpid(pid, &status, 0);
            // printf("Child process %d has terminated.\n", pid);
        } 
        else {
            // If running in the background, don't wait for the child process
            printf("Running in the background : pid = [%d]\n", pid);
        }
    } 
    else {
        // Fork failed
        perror("fork");
        exit(EXIT_FAILURE);
    }

    return child_info;
}

int is_background(char **args) {
    int i = 0;
    while (args[i] != NULL) {
        if (strcmp(args[i], "&") == 0) {
            return 1;       
        }
        i++;
    }
    return 0; 
}

int process(char **args) {
    int rv = 0;
    if (args[0] == NULL) 
        rv = 0;
    
    else if (is_control_command(args[0])) 
        rv = do_control_command(args);
    
    else if (ok_to_excute()) {
        int background = is_background(args);

        // Check for multiple commands separated by '&'
        int i = 0;
        while (args[i] != NULL) {
            if (strcmp(args[i], "&") == 0) {
                args[i] = NULL;  
                excute_command(args, background);
                args += i + 1;  
                i = 0;            
                continue;
            }
            i++;
        }

        // Excute the last command
        excute_command(args, 0);
    }
    return rv;
}


