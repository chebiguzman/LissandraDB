scheduler_config* config;
t_queue* exec_queue;

int exec_size = 0;
void* exec(void *system_queue){
    exec_queue = queue_create();
  
    config = malloc(sizeof(scheduler_config));
    
    
    bool superuser = false;
    log_debug(logg, "se inicia el modulo exec");
    while(true){

        pthread_mutex_lock(&config_lock);
        config->multi_script_level = config_not->multi_script_level;
        config->quantum = config_not->quantum;
        pthread_mutex_unlock(&config_lock);
       
       //printf("el nuvel de procesamiento es:%d, y quandum:%ld", config->multi_script_level, config->quantum);

        while(!queue_is_empty( exec_queue ) && exec_size != 0){
            ///obtengo el proximo programa de la cola de exec
            EXECUTION:
            log_debug(logg, "exec:obtengo programa");
            t_instr_set* program;
            t_ksyscall* kernel_call;

            /*Reviso si hay llamadas al sistema
            si las hay las pongo primeras en linea para ejecutar en el proximo
            cuantum ya que RR es apropiativo en el quantum*/

            pthread_mutex_lock(&syscall_queue->lock);
            if(!queue_is_empty(syscall_queue->scheduler_queue)){
                log_debug(logg, "Se recibe un syscall");
                kernel_call = queue_pop(syscall_queue->scheduler_queue);
                program = kernel_call->instr;
                superuser = true;
            }else{
                program = queue_pop(exec_queue);
                superuser = false;
            }
            pthread_mutex_unlock(&syscall_queue->lock);


            for(int i = 0; i != config->quantum; i++){
            
                char* instr = strdup(queue_pop( program->instr));

                //en un RUN los comandos se van mostrando
                //a medida que ejecutan
                if(program->doesPrint){
                    printf("%s",instr);
                }
                ///home/dreamable/a.lql
                //log_debug(logg, instr);
                char* r = exec_instr(instr);
                //log_debug(logg, "exec:obtengo respuesta");
                printf("%s", r);

                
                if(superuser){
                    kernel_call->result = r;
                    pthread_cond_broadcast(&kernel_call->cond);
                    log_debug(logg, "se le debvuelve al kernel su pedido");
                }
                //free(r);
                free(instr);

                /* si el programa se termino de ejecutar
                voy a buscar otro para mantener el nivel de 
                multiprogramacion, si no, lo devuelvo a exec*/
                if(queue_is_empty( program->instr)){
                    log_debug(logg, "exec:termino programa");
                    
                    exec_size--;
                   
                    lock_queue();
                    log_debug(logg, "exec:me voy a buscar otro programa");
                    updateTasks(exec_queue);
                    
                    break;
                }


            }

            if(!queue_is_empty( program->instr)){
                log_debug(logg, "exec: termino quantum");
                queue_push(exec_queue, program);
            }else{
                queue_destroy(program->instr);
                free(program);
            }
            
        }

             

        
        
        syscall_availity_status = true;
        //devuelvo el control a consola
        pthread_mutex_unlock(&console->lock);
        pthread_cond_broadcast(&console->cond);

         /*si mi cola de exec se quedo  vacia quiere decir
        que tambien la del scheduler asi que espero una señal
        ya sea una syscal o que añadan una tarea*/ 
        
        pthread_cond_wait(&queue->cond, &queue->lock);
        //log_debug(logg, "exec:me llego una query");
        if(!queue_is_empty(syscall_queue->scheduler_queue)){
            log_debug(logg, "Una syscall libera al procesador ocioso");
           goto EXECUTION;
            
        }else{
            lock_queue();
            updateTasks(exec_queue);
        }
        
        
    }   

    
    
}

//obtiene de la cola del scheduler las proximas tareas
//la cola ya debe estar bloqueada
void updateTasks(t_queue* q){

   // l//og_debug(logg, "exec:actualizando cola de exec");
    while(exec_size != config->multi_script_level && !queue_is_empty(queue->scheduler_queue)){
        
        t_instr_set* new_program = queue_pop(queue->scheduler_queue);
        //log_debug(logg, "nueva instruccion:");
        //log_debug(logg, queue_peek(new_program->instr));
        queue_push(exec_queue, new_program);
        exec_size++;
        //log_debug(logg, "exec:agrego un programa");
        
        
    }
    unlock_queue();
    //log_debug(logg, "exec:fin de acctualizacion");

    
}
