/* mshell - a job manager */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>


#include "jobs.h"
#include "common.h"
#include "sighandlers.h"

/*
 * wrapper for the sigaction function
 */
int sigaction_wrapper(int signum, handler_t * handler) {

    struct sigaction tmp;
    int status; 
    
    tmp.sa_handler=handler;
    sigemptyset(&(tmp.sa_mask));
    tmp.sa_flags = SA_RESTART;
    status = sigaction(signum,&tmp,NULL);

    if (status == -1) {
        unix_error("sigaction error");
    }

    return 1;
}

/*
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children
 */
void sigchld_handler(int sig) {

    int status;
    pid_t pid_f;

    if (verbose) {
        printf("Entering sigchld_handler\n");
    }

    pid_f = waitpid(-1,&status,WUNTRACED);
    jobs_pid2jid(pid_f);

    if (WIFSIGNALED(status)) {
        jobs_deletejob(pid_f);
        if (verbose) {
            printf("Signal non intercepté\n");
        }
    }

    if (WIFEXITED(status)) {
        jobs_deletejob(pid_f);
        if (verbose) {
            printf("Signal terminé \n");
        }
    }

    if (WIFSTOPPED(status)) {

        struct job_t *tmp;

        tmp = jobs_getjobpid(pid_f);
        tmp->jb_state = ST;
        

        if (verbose) {
            printf("Signal arrêté\n");
        }
        
    }

    if (verbose)
        printf("fin de sigchld_handler\n");

    return;
}

/*
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.
 */
void sigint_handler(int sig) {

    pid_t pid;
    int status;

    if (verbose) {
        fprintf(stderr, "Existing sigint_handler %d\n",sig );
    }

    if ((pid = jobs_fgpid()) >0) {

        status = kill(pid,SIGINT);

        if (status <0) {
            unix_error("kill error");
        };

        if (verbose) {
            printf("J'ai tué le signal\n");
        }

    }

    else {
        if (verbose) {
            printf("Pas de fg jobs  \n");
        }
    }

    return;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.
 */
void sigtstp_handler(int sig) {

    pid_t pid;
    int status; 

    if (verbose) {
        fprintf(stderr, "Existing sigint_handler %d\n",sig);
    }

    if ((pid = jobs_fgpid()) > 0) {

        status = kill(pid,SIGTSTP); 

        if (status< 0) {
            unix_error("kill error");
        }

        if (verbose) {
            printf("J'ai suspendu le signal\n");
        }
    }

    else {

        if (verbose) {
            printf("pas de fg jobs\n");
        }
    }
    return;
}
