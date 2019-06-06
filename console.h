#include <pthread.h>

void *console_input(void *no_args);
void *console_input_wait(void *no_args);

typedef struct{
    pthread_mutex_t lock;
    pthread_cond_t cond;
    char* name;
}t_console_control;
