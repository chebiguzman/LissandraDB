scheduler_config* config;
t_queue* exec_queue;
int exec_size = 0;
void* exec(void *sch_queue){
    exec_queue = queue_create();
    config = malloc(sizeof(scheduler_config));
    
    

    log_debug(logg, "se inicia el modulo exec");
    while(true){

        pthread_mutex_lock(&config_lock);
        config->multi_script_level = config_not->multi_script_level;
        config->quantum = config_not->quantum;
        pthread_mutex_unlock(&config_lock);
       
       printf("el nuvel de procesamiento es:%d, y quandum:%ld", config->multi_script_level, config->quantum
       );

        while(!queue_is_empty( exec_queue ) && exec_size != 0){
            ///obtengo el proximo programa de la cola de exec
            log_debug(logg, "exec:obtengo programa");
            t_instr_set* program = queue_pop(exec_queue);

            for(int i = 0; i != config->quantum; i++){
                log_error(logg, string_itoa(i));
            
                char* instr = strdup(queue_pop( program->instr));

                //en un RUN los comandos se van mostrando
                //a medida que ejecutan
                if(program->doesPrint){
                    printf("%s",instr);
                }
                ///home/dreamable/a.lql
                log_debug(logg, instr);
                char* r = exec_instr(instr);
                log_debug(logg, "exec:obtengo respuesta");
                printf("%s", r);


                //free(r);
                //free(instr);

                /* si el programa se termino de ejecutar
                voy a buscar otro para mantener el nivel de 
                multiprogramacion, si no, lo devuelvo a exec*/
                if(queue_is_empty( program->instr)){
                    log_debug(logg, "exec:termino programa");
                    exec_size--;
                    log_debug(logg, "exec:me voy a buscar otro programa");
                    lock_queue();
                    updateTasks(exec_queue);
                    queue_destroy(program->instr);
                    //free(program);
                    break;
                }


            }

            if(!queue_is_empty( program->instr)){
                log_debug(logg, "exec: termino quantum");
                queue_push(exec_queue, program);
            }
            
        }

        /*si mi cola de exec se quedo  vacia quiere decir
        que tambien la del scheduler asi que espero una señal*/       
        log_debug(logg, "exec:cola exec vacia");
        
        
        log_debug(logg, "exec:devuelvo control a consola");
        //devuelvo el control a consola
        pthread_mutex_unlock(&console->lock);
        pthread_cond_broadcast(&console->cond);

        lock_queue();
        pthread_cond_wait(&queue->cond, &queue->lock);
        log_debug(logg, "exec:me llego una query");
        updateTasks(exec_queue);
        
    }   

    
    
}

//obtiene de la cola del scheduler las proximas tareas
//la cola ya debe estar bloqueada
void updateTasks(t_queue* q){

    log_debug(logg, "exec:actualizando cola de exec");
    while(exec_size != config->multi_script_level && !queue_is_empty(queue->scheduler_queue)){
        
        t_instr_set* new_program = queue_pop(queue->scheduler_queue);
        log_debug(logg, "nueva instruccion:");
        log_debug(logg, queue_peek(new_program->instr));
        queue_push(exec_queue, new_program);
        exec_size++;
        log_debug(logg, "exec:agrego un programa");
        
        
    }
    unlock_queue();
    log_debug(logg, "exec:fin de acctualizacion");

    
}
