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
#include <unistd.h>
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

scheduler_config* config_not;
pthread_mutex_t config_lock = PTHREAD_MUTEX_INITIALIZER;
bool syscall_availity_status = false; //No puedo hacer una sys call hasta que algun exec llegue a una espera.
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

    int m = config_get_int_value(fconfig, "MULTIPROCESAMIENTO");
    long ref = config_get_long_value(fconfig, "METADATA_REFRESH");
    config_not->multi_script_level = m;
    config_not->quantum = 1;
    config_not->metadata_refresh = ref;
    
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
    inotify_add_watch(inotifyFd, "config", IN_CLOSE_WRITE);
    char* buf = malloc(EVENT_BUF_LEN);
    while(1){
        int length = read(inotifyFd, buf, EVENT_BUF_LEN);

         if ( length < 0 ) {
            perror( "Error en config" );
        }  

        struct inotify_event *event = (struct inotify_event *) buf;
        if(event->mask == IN_CLOSE_WRITE){
        //config_destroy(fconfig);
        fconfig = config_create("config");
        log_info(logg, "ConfigWorker: se modifico el archivo de configuracion");
        update_scheduler_config();

        }
    }
    
}

//leo y actualico la informaion del config;
void update_scheduler_config(){
         
        int q = config_get_int_value(fconfig, "QUANTUM");
        long sleep = config_get_long_value(fconfig, "SLEEP_EJECUCION");
        long refresh = config_get_long_value(fconfig, "METADATA_REFRESH");
        //log_debug(logg, "el nuevo es: ");
        //log_debug(logg, string_itoa((int) refresh));
        pthread_mutex_lock(&config_lock);
        config_not->quantum = q;
        config_not->sleep = sleep;
        config_not->metadata_refresh = refresh;
        pthread_mutex_unlock(&config_lock);
}

#include "exec.c"

void lock_queue(){
    //log_info(logg, "scheduler: BLOQUEO COLA DE PROCESOS");
    pthread_mutex_lock(&queue->lock);
}
void unlock_queue(){
    pthread_mutex_unlock(&queue->lock);
    //log_info(logg, "scheduler: DESBLOQUEO COLA DE PROCESOS");

}


void schedule(t_instr_set* instr_set){
    lock_queue();
    queue_push(queue->scheduler_queue, instr_set);
    //log_info(logg, "sh:acrego una instruccion");
    unlock_queue();
    
    pthread_cond_broadcast(&queue->cond);
    //log_info(logg, "llame a exec");
}

char* ksyscall(char* call){

    if(syscall_availity_status){
        log_debug(logg, "syscall");
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

        
        
        pthread_cond_broadcast(&queue->cond);
        pthread_mutex_lock(&syscall->lock);
        pthread_cond_wait(&syscall->cond, &syscall->lock);
        
        char* res = strdup(syscall->result);
        pthread_mutex_destroy(&syscall->lock);
        pthread_cond_destroy(&syscall->cond);
        
        free(syscall);

        return res;

    }else{
        log_debug(logg, "Aun no estan disponibles las syscall");
        return "";
    }
}


