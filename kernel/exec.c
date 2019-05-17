#include <pthread.h>
#include "scheduler.h"
#include "../pharser.h"

void exec(void* sch_queue){
    t_queue* exec_queue = queue_create();
    scheduler_queue* queue = sch_queue;
    

    while(!queue_is_empty( exec_queue ) ){
        instr_set* program = queue_pop(exec_queue);
        
        for(int i = 0; i <= 1; i++){

        
            char* instr = queue_pop( program->instr);

            if(program->doesPrint){
                printf(instr);d
            }

            char* r = exec_instr(instr);
            printf("%s", r);
            free(r);
            free(instr);

            if(queue_is_empty( program->instr)){
                updateTasks();
                queue_destroy(program->instr);
                free(program);
                break;
            }else{
                queue_push(exec_queue, program);
            }


        }
        
    }
        

    
    
}