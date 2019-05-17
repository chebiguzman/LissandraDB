#include <commons/collections/queue.h>
#include <pthread.h>
#include <commons/log.h>
typedef struct 
{
    t_queue* instr;
    int doesPrint;
}t_instr_set;



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
typedef struct 
{
    t_log* log;
    scheduler_queue** queue;
}exec_atrr;

void schedule(t_instr_set* instr_set);
void start_sheduler();
void lock_queue();
void unlock_queue();
void scheduler_queue_create(scheduler_queue** s);
void schedule(t_instr_set* instr_set);
void ksyscall(char* call);
void* exec();
void updateTasks(t_queue* exec_queue, int exec_size);