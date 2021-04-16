#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include "Test_Watchdog.h"

struct timeval counter, watch;
pid_t child = 0;
pthread_t threads;
int thread_args;

void callback(){
    gettimeofday(&counter, NULL);
}

void end(){
    printf("end\n");
    exit(0);
}

void *watch_dog(){
    while(1){
        if(&counter){
            sleep(1);
            printf("watch: %lus %lums, counter: %lus %lums, diff: %lus %lums\n",watch.tv_sec,watch.tv_usec, counter.tv_sec,counter.tv_usec, watch.tv_sec - counter.tv_sec, watch.tv_usec - counter.tv_usec); 
            gettimeofday(&watch, NULL);
            printf("watch: %lus %lums, counter: %lus %lums, diff: %lus %lums\n",watch.tv_sec,watch.tv_usec, counter.tv_sec,counter.tv_usec, watch.tv_sec - counter.tv_sec, watch.tv_usec - counter.tv_usec); 
            if(watch.tv_sec - counter.tv_sec>3){
                end();
                break;
            }
        }
    }
    while(1){
        sleep(1);
        printf("still alive\n");
    }

    return 0;
}

void catchSig(){
    printf("CAUGHT\n");
    end();
}

int main(){
    int i;
    
    for(i = 1; i <=SIGRTMIN ; i++){
        //if(i != SIGINT && i != SIGTSTP) signal(i,catchSig);
    }

    pthread_create(&threads, NULL, watch_dog, NULL);

    for(i=0;i<10;i++){
        sleep(1);
        callback();
    }
    //end();
    while(1){
        
    }
    
    printf("end_main\n");
    //pthread_join(threads, NULL);

    return 0;
}