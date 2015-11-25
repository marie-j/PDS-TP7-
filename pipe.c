/* mshell - a job manager */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "pipe.h"
#include "jobs.h"
#include "cmd.h"

void do_pipe(char *cmds[MAXCMDS][MAXARGS], int nbcmd, int bg) {

    int fd[2][2], i;
    pid_t pid;

    if (verbose) printf("Entering pipe\n");

    if (verbose) printf("Pipe creation\n");


    if (pipe(fd[0]) == -1) {

      if (verbose) printf("Error in pipe creation\n");
    }

    for (i = 0; i < nbcmd; i++) {

      if (i == 0) {

        switch(pid = fork()) {
          case -1 :

            if (verbose) unix_error("fork cmd 1 error");

          case 0 :
            if (verbose) printf("First son process\n");

            setpgid(0, 0);
            dup2(fd[0][1],STDOUT_FILENO);

            close(fd[0][0]);
            close(fd[0][1]);

            execvp(cmds[0][0],cmds[0]);

            printf("execvp error\n");
            exit(EXIT_FAILURE);

          default :
            if (verbose) printf("Father process\n");
        }
      }

      else if (i > 0 && i < nbcmd -1) {

        switch(pid = fork()) {
          case -1 :
            unix_error("fork cmd 2 error");

          case 0 :

            if (verbose) printf("the %dth process\n",i);

            assert(pipe(fd[i%2]) != -1);

            setpgid(0, pid);
            dup2(fd[(i+1)%2][0], STDIN_FILENO);
            dup2(fd[i%2][1], STDOUT_FILENO);
            close(fd[0][0]);
            close(fd[0][1]);
            close(fd[1][1]);
            close(fd[1][0]);
            execvp(cmds[i][0],cmds[i]);
            printf("execvp 2 error\n");
            exit(EXIT_FAILURE);

          default :

            close(fd[(i+1)%2][0]);
            close(fd[(i+1)%2][1]);
        }
      }

      else {
        switch(pid = fork()) {
          case -1 :
            unix_error("fork cmd 2 error");

          case 0 :
            if (verbose) {
                printf("Last process\n");
            }
            dup2(fd[(i+1)%2][0], STDIN_FILENO);
            close(fd[(i+1)%2][0]);
            close(fd[(i+1)%2][1]);
            execvp(cmds[i][0],cmds[i]);
            printf("execvp 2 error\n");
            exit(EXIT_FAILURE);

          default:

            close(fd[(i+1)%2][0]);
            close(fd[(i+1)%2][1]);
        }
      }

    }

    jobs_addjob(pid, (bg == 1 ? BG : FG), cmds[0][0]);

    if (!bg) {
      if (verbose) printf("pipe waiting fg \n");

      waitfg(pid);
    }

    return;
}
