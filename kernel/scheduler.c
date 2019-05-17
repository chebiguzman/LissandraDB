#include <commons/collections/queue.h>

#include "scheduler.h"
#include<pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <stdio.h>
#include <stdlib.h>
#include "../pharser.h"
scheduler_status* status;
scheduler_queue* queue;
t_log* logg;
void start_sheduler(t_log* log){
    log_info(log, "se inicia el modulo scheduler");
    logg = log;
    status= malloc(sizeof(scheduler_status));
    status->multi_script_level = 1;
    status->quantum = 1;
    
    queue = malloc(sizeof(scheduler_queue));
    queue->scheduler_queue = queue_create();
    int r = pthread_mutex_init(&queue->lock, NULL);
    pthread_cond_init(&queue->cond,NULL);
    //start service innotifyi

    pthread_t tid;
    pthread_create(&tid, NULL, exec, NULL);

    
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

void ksyscall(char* call){

}

