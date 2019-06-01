#include <commons/collections/queue.h>

#include "scheduler.h"
#include<pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include "../pharser.h"
#include "../console.h"
#include <sys/types.h>

#include <sys/inotify.h>
#include <errno.h>
#include <commons/config.h>

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

scheduler_config* config_not;
pthread_mutex_t config_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t config_cond = PTHREAD_COND_INITIALIZER;

scheduler_queue* queue;
scheduler_queue* syscall_queue;

t_log* logg;
t_console_control* console;
t_config* fconfig;

void start_sheduler(t_log* log, t_console_control* console_control ){

    log_info(log, "se inicia el modulo scheduler");

    fconfig = config_create("config");
    logg = log;
    config_not= malloc(sizeof(scheduler_config));
    config_not->multi_script_level = 1;
    config_not->quantum = 1;
    
    update_scheduler_config();

    pthread_t worker_tid;
    pthread_create(&worker_tid,NULL,config_worker, NULL);
    
    syscall_queue = malloc( sizeof(scheduler_queue));
    pthread_mutex_init(&syscall_queue->lock, NULL);
    syscall_queue->scheduler_queue = queue_create();

    queue = malloc(sizeof(scheduler_queue));
    queue->scheduler_queue = queue_create();
    int r = pthread_mutex_init(&queue->lock, NULL);
    pthread_cond_init(&queue->cond,NULL);
    
    console = console_control;
    pthread_t tid;
    pthread_create(&tid, NULL, exec, NULL);

    
}

void* config_worker(void* args){
    log_info(logg, "se inicia el monitoreo del config");
    int inotifyFd = inotify_init();
    inotify_add_watch(inotifyFd, "config", IN_MODIFY);
    char* buf = malloc(3000);
    while(1){
        read(inotifyFd, buf, 3000);
        struct inotify_event *event = (struct inotify_event *) buf;
        if(event->mask == IN_MODIFY){
        log_info(logg, "se modifico el archivo de configuracion");
        update_scheduler_config();

    }
    }
    
}

//leo y actualico la informaion del config;
void update_scheduler_config(){
         
        pthread_mutex_lock(&config_lock);
        
        int q = config_get_int_value(fconfig, "QUANTUM");
        int m = config_get_int_value(fconfig, "MULTIPROCESAMIENTO");
        config_not->quantum = q;
        config_not->multi_script_level = m;
        
        pthread_mutex_unlock(&config_lock);
        pthread_cond_broadcast(&config_cond);
}

#include "exec.c"

void lock_queue(){
    pthread_mutex_lock(&queue->lock);
}
void unlock_queue(){
    pthread_mutex_unlock(&queue->lock);
}


void schedule(t_instr_set* instr_set){
    lock_queue();
    queue_push(queue->scheduler_queue, instr_set);
    log_info(logg, "sh:acrego una instruccion");
    unlock_queue();
    
    pthread_cond_broadcast(&queue->cond);
    log_info(logg, "llame a exec");
}

char* ksyscall(char* call){
    t_ksyscall* syscall = malloc( sizeof(t_ksyscall));
    syscall->instr = malloc ( sizeof ( t_instr_set));

    t_queue* kqueue = queue_create();
    queue_push(kqueue, call);

    syscall->instr->instr = kqueue;
    syscall->instr->doesPrint = false;

    pthread_mutex_init(&syscall->lock, NULL);
    pthread_cond_init(&syscall->cond, NULL);

    pthread_mutex_lock(&syscall_queue->lock);
    queue_push(syscall_queue->scheduler_queue, syscall);
    pthread_mutex_unlock(&syscall_queue->lock);

    pthread_mutex_lock(&syscall->lock);
    pthread_cond_wait(&syscall->cond, &syscall->lock);

    char* res = strdup(syscall->result);
    pthread_mutex_destroy(&syscall->lock);
    pthread_cond_destroy(&syscall->cond);
    free(syscall);

    return res;
}

