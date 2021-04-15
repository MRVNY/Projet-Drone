#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include "Test_Watchdog.h"

//time_t counter = 0;
//time_t watch = 0;
clock_t counter = 0;
clock_t watch = 0;
pid_t child = 0;
pthread_t threads;
int thread_args;

void callback(){
    //time(&counter);
    counter = clock();
    /*struct timeval stop, start;
gettimeofday(&start, NULL);
//do stuff
gettimeofday(&stop, NULL);
printf("took %lu us\n", (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec); */
}

void end(){
    counter = 0;
    printf("end\n");
}

void *watch_dog(){
    while(1){
        if(counter!=0){
            sleep(1);
            //time(&watch);
            watch = clock();
            printf("watch:%ld counter:%ld, diff:%f\n",watch,counter,difftime(watch,counter));
            if(watch-counter>20){
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
    printf("can't die\n");
}

int main(){
    int i;
    
    for(i = 1; i <=SIGRTMIN ; i++){
        if(i != SIGINT && i != SIGTSTP) signal(i,catchSig);
    }

    pthread_create(&threads, NULL, watch_dog, NULL);

    for(i=0;i<10;i++){
        sleep(1);
        callback();
    }
    //end();
    while(1){
        if(counter==0) break;
    }
    
    printf("end_main\n");
    //pthread_join(threads, NULL);

    return 0;
}