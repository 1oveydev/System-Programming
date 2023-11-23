#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include "smsh.h"

#define DFL_PROMPT "> "

int VLenviron2table(char *env[]);

int main() {
    char *cmdline, *prompt, **arglist;
    int result;
    void setup();

    prompt = DFL_PROMPT;
    setup();

    while ((cmdline = next_cmd(prompt, stdin)) != NULL) {
        if ((arglist = splitline(cmdline)) != NULL) {
            // Check for "exit" command
            if (strcmp(arglist[0], "exit") == 0) {
                freelist(arglist);
                free(cmdline);
                exit(0);
            }

            result = process(arglist);
            waitpid(-1, NULL, WNOHANG);
            
            freelist(arglist);
        }
        free(cmdline);
    }
    return 0;
}

void setup() {
    extern char **environ;

    VLenviron2table(environ);
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
}

void fatal(char *s1, char *s2, int n) {
    fprintf(stderr, "Error: %s, %s\n", s1, s2);
    exit(n);
}

