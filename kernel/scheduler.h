#include <commons/collections/queue.h>

typedef struct 
{
    t_queue* instr;
    int doesPrint;
}instr_set;

typedef struct 
{
    t_queue* scheduler_queue;
    pthread_mutex_t lock;
    pthread_cond_t cond;
}scheduler_queue;


void lock_queue();
void unlock_queue();
