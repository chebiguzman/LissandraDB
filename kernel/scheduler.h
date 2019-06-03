#include <commons/collections/queue.h>
#include <pthread.h>
#include <commons/log.h>
#include <string.h>
typedef struct 
{
    t_queue* instr;
    int doesPrint;
}t_instr_set;

typedef struct
{
    t_instr_set* instr;
    char* result;
    pthread_mutex_t lock;
    pthread_cond_t cond;
}t_ksyscall;


typedef struct 
{
    long quantum;
    long sleep;
    long metadata_refresh;
    int multi_script_level;


}scheduler_config;

typedef struct 
{
    t_queue* scheduler_queue;
    pthread_mutex_t lock;
    pthread_cond_t cond;
}scheduler_queue;
typedef struct 
{
    t_log* log;
    scheduler_queue** queue;
}exec_atrr;

extern scheduler_config* config_not;
extern pthread_mutex_t config_lock;

void schedule(t_instr_set* instr_set);
void start_sheduler();
void lock_queue();
void unlock_queue();
void scheduler_queue_create(scheduler_queue** s);
void schedule(t_instr_set* instr_set);
char* ksyscall(char* call);
void* exec();
void updateTasks(t_queue* exec_queue);
void* config_worker(void* args);
void update_scheduler_config();