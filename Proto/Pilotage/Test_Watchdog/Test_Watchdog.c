#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>

time_t counter = 0;
time_t watch = 0;
pid_t child = 0;
pthread_t threads;
int thread_args;

void callback(){
    time(&counter);
}

void end(){
    counter = 0;
    printf("end\n");
}

void *watch_dog(){
    while(1){
        if(counter!=0){
            sleep(1);
            time(&watch);
            printf("watch:%ld counter:%ld\n",watch,counter);
            if(watch-counter>3){
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

void catch(){
    printf("can't die\n");
}

int main(){
    int i = 0;
    signal(SIGINT,catch);
    signal(SIGSEGV,catch);
    //signal(SIGTSTP,catch);
    pthread_create(&threads, NULL, watch_dog, NULL);

    raise(SIGSEGV); //Segmentation fault

    for(i=0;i<5;i++){
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