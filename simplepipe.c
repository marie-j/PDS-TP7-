#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "pipe.h"

void do_pipe(char *cmds[MAXCMDS][MAXARGS], int nbcmd, int bg) {
    
    int fd[2],res,status;

    if (verbose) {
        printf("Entering pipe\n");
    }

    if (verbose) {
        printf("Pipe creation\n");
    }
    res = pipe(fd);
    if (res == -1) {
        unix_error("Error in pipe creation");
    }




    switch(fork()) {
        case -1 : 
            unix_error("fork cmd 1 error");

        case 0 : 
            if (verbose) {
                printf("First son process\n");
            }
            dup2(fd[1],STDOUT_FILENO);
            close(fd[0]);
            execvp(cmds[0][0],cmds[0]);
            printf("execvp error\n");
            exit(EXIT_FAILURE);

        default : 
            if (verbose) {
                printf("Father process\n");
            }
        ;
    }

    switch(fork()) {
        case -1 :
            unix_error("fork cmd 2 error");

        case 0 : 
            if (verbose) {
                printf("Second son process\n");
            }
            close(fd[1]);
            dup2(fd[0],STDIN_FILENO);
            execvp(cmds[1][0],cmds[1]);
            printf("execvp 2 error\n");
            exit(EXIT_FAILURE);

        default : ; 
    }

    close(fd[0]);
    close(fd[1]);
    wait(&status);

    if (WIFEXITED(status)) {
        res = WEXITSTATUS(status);
    }
    else {
        res = -1;
    }

    wait(&status);

    if (WIFEXITED(status)) {
        res |= WEXITSTATUS(status);
    }
    else {
        res = -1;
    }
    return;
}
