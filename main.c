#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#define FOR(i, k) for(int i=0; i<k; i++)

int N = 0; // line count
pthread_t tid[512];

int c_sleep[512] = {0};
char c_cmd[512][160]; // 512 max lines, with max 160 chars
int c_timerules[512][5];

int execable(int *timerules){
    struct tm *tm;
    time_t tim = time(NULL);
    tm = localtime(&tim);

    int i=0, flag=1, fdom=0;
    // perhatikan 2 sama 4
    // rule 4 matikan dulu
    if(timerules[0] != tm->tm_min && timerules[0] != -1) flag = 0;
    if(timerules[1] != tm->tm_hour && timerules[1] != -1) flag = 0;
    if(timerules[2] != tm->tm_mday && timerules[2] != -1) {
        flag = 0;
        fdom = 1; // tidak sesuai tgl, tapi lihat weekday nanti
    }
    if(timerules[3] != tm->tm_mon && timerules[3] != -1) flag = 0;
    if(timerules[4] != tm->tm_wday && timerules[4] != -1) flag = 0;
    if(timerules[4] == tm->tm_wday && fdom) flag = 1; // reverse fdom effect

    if(tm->tm_sec != 0) flag = 0;

    // printf("\n");
    return flag;
}

void* execute(void *arg){
    int i, f=0;
    pthread_t id = pthread_self();
    pid_t child=0;
    // printf("s\n");
    for(i=0; i<N; i++){
        // printf("%d %d %d\n", i, id, tid[i]);
        if((id == tid[i]) && (execable(c_timerules[i]))){
            // printf("\n%d: %s\n", i, c_cmd[i]);
            child = fork();

            if(child == 0){
                execl("/bin/bash", "bash", "-c", c_cmd[i], NULL);
            }
        }
    }
}

int main() {
    pid_t pid, sid;

    pid = fork();

    if (pid < 0) {
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    umask(0);

    sid = setsid();

    if (sid < 0) {
        exit(EXIT_FAILURE);
    }

    // if ((chdir("/")) < 0) {
    //     exit(EXIT_FAILURE);
    // }

    close(STDIN_FILENO);
    // close(STDOUT_FILENO);
    // close(STDERR_FILENO);

    FILE * cronconfig;
    char buffer[100];

    while(1) {
        // main program here
        int linecnt=0, err;
        cronconfig = fopen("crontab.data", "r");

        if (!cronconfig)
        {
            perror("Opening cronconfig");
            return (-1);
        }

        while (EOF != fscanf(cronconfig, "%100[^\n]\n", buffer)) {
            char *token = strtok(buffer, " ");
            int flag=0;

            strcpy(c_cmd[linecnt], "");
            while(token) {
                if(flag < 5) {
                    if(!strcmp(token, "*")) c_timerules[linecnt][flag] = -1;
                    else c_timerules[linecnt][flag] = atoi(token);  
                } else {
                    strcat(c_cmd[linecnt], token);
                    strcat(c_cmd[linecnt], " ");
                }
                token = strtok(NULL, " ");
                flag++;
            }

            linecnt++;
        }

        N = linecnt;

        FOR(i, N) err = pthread_create(&(tid[i]), NULL, execute, NULL);

        fclose(cronconfig);
        sleep(1);

        FOR(i, N) err = pthread_cancel(tid[i]); 
    }

    exit(EXIT_SUCCESS);
}