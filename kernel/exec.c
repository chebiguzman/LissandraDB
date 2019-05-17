
void* exec(void *sch_queue){
    t_queue* exec_queue = queue_create();
    int exec_size = 0;


    log_info(logg, "se inicia el modulo exec");
    while(true){
        
        while(!queue_is_empty( exec_queue ) ){
            ///obtengo el proximo programa de la cola de exec
            log_info(logg, "exec:obtengo programa");
            t_instr_set* program = queue_pop(exec_queue);
            
            for(int i = 0; i <= status->quantum; i++){

            
                char* instr = queue_pop( program->instr);

                //en un RUN los comandos se van mostrando
                //a medida que ejecutan
                if(program->doesPrint){
                    printf("%s",instr);
                }

                log_info(logg, "exec:iteracion r");
                char* r = exec_instr(instr);
                log_info(logg, "exec:obtengo respuesta");
                printf("%s", r);
                //free(r);
                //free(instr);

                /* si el programa se termino de ejecutar
                voy a buscar otro para mantener el nivel de 
                multiprogramacion, si no, lo devuelvo a exec*/
                if(queue_is_empty( program->instr)){
                    log_info(logg, "exec:queue de programa vacia");
                    exec_size--;
                    log_info(logg, "exec:me voy a buscar otra");
                    lock_queue();
                    updateTasks(exec_queue, exec_size);
                    //queue_destroy(program->instr);
                    //free(program);
                    break;
                }else{
                    queue_push(exec_queue, program);
                }


            }
            
        }

        /*si mi cola de exec se quedo  vacia quiere decir
        que tambien la del scheduler asi que espero una señal*/       
        log_info(logg, "exec:cola exec vacia");
        lock_queue();
        log_info(logg, "exec:me quedo esperando una queriy");
        pthread_cond_wait(&queue->cond, &queue->lock);
        log_info(logg, "exec:me llego una query");
        updateTasks(exec_queue, exec_size);
    }   

    
    
}

//obtiene de la cola del scheduler las proximas tareas
//la cola ya debe estar bloqueada
void updateTasks(t_queue* exec_queue, int exec_size){
    log_info(logg, "exec:update afuera de while");
    log_info(logg, "llamo a exec");
    while(exec_size < status->multi_script_level && !queue_is_empty(queue->scheduler_queue)){
        log_info(logg, "exec:update afuera de whileadentro");
        t_instr_set* new_program = queue_pop(queue->scheduler_queue);
        log_info(logg, "update:");
        log_info(logg, queue_peek(new_program->instr));
        queue_push( exec_queue, new_program);
        exec_size++;
        log_info(logg, "exec:agrego un programa");
    }
    unlock_queue();
    log_info(logg, "exec:termino update");
}
