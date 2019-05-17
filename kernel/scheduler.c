#include <commons/collections/queue.h>

#include "scheduler.h"
#include<pthread.h>

typedef struct 
{
    long quantum;
    int multi_script_level;

}scheduler_status;

typedef struct 
{
    t_queue* scheduler_queue;
    pthread_mutex_t lock;
    pthread_cond_t cond;
}scheduler_queue;


scheduler_status* status;
scheduler_queue* queue;

void start_sheduler(){
    status= malloc(sizeof(scheduler_status));
    status->multi_script_level = 1;
    status->quantum = 1;

    queue = scheduler_queue_create();
    //start service innotifyi


    
}

scheduler_queue* scheduler_queue_create(){
    scheduler_queue* q = malloc(sizeof(scheduler_queue));
    queue_create(queue->scheduler_queue);
    pthread_mutex_init(&queue->lock, NULL);
    pthread_cond_init(&queue->cond);

}
void lock_queue(){
    pthread_mutex_lock(&queue->lock);
}
void unlock_queue(){
    pthread_mutex_unlock(&queue->lock);
}


void schedule(instr_set* instr_set){
    lock_queue();
    queue_push(queue->scheduler_queue, instr_set);
    unlock_queue();
    pthread_cond_broadcast(&queue->cond);

}

void syscall(char* call){

}

