#include <pthread.h>

void *console_input(void *no_args);
void *console_input_wait(void *no_args);

typedef struct{
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int cont_int;
    char* name;
    pthread_mutex_t print_lock;
}t_console_control;
