scheduler_config* config;
t_queue* exec_queue;
__thread int err_trap = 0;
int exec_size = 0;
void* exec(void *system_queue){
    exec_queue = queue_create();
  
    config = malloc(sizeof(scheduler_config));
    
    
    bool superuser = false;
    log_debug(logger_debug, "se inicia el modulo exec");
    while(true){

        pthread_mutex_lock(&config_lock);
        config->multi_script_level = config_not->multi_script_level;
        config->quantum = config_not->quantum;
        pthread_mutex_unlock(&config_lock);
       
       //printf("el nuvel de procesamiento es:%d, y quandum:%ld", config->multi_script_level, config->quantum);

        while(!queue_is_empty( exec_queue )){
            ///obtengo el proximo programa de la cola de exec

            EXECUTION:
            err_trap = 0; //la label necesita una linea denajo de ella
            //log_debug(logger_debug, "exec:obtengo programa");
            t_instr_set* program;
            t_ksyscall* kernel_call;

            /*Reviso si hay llamadas al sistema
            si las hay las pongo primeras en linea para ejecutar en el proximo
            cuantum ya que RR es apropiativo en el quantum*/

            pthread_mutex_lock(&syscall_queue->lock);
            if(!queue_is_empty(syscall_queue->scheduler_queue)){
                //log_debug(logg, "Se recibe un syscall");
                kernel_call = queue_pop(syscall_queue->scheduler_queue);
                program = kernel_call->instr;
                superuser = true;
            }else{
                program = queue_pop(exec_queue);
                superuser = false;
            }
            pthread_mutex_unlock(&syscall_queue->lock);


            for(int i = 0; i != config->quantum; i++){
                //log_debug(logger_debug, "exec:me preparo para ejecutar");
                
                char* instr = strdup(queue_pop( program->instr));
                //en un RUN los comandos se van mostrando
                //a medida que ejecutan
                if(program->doesPrint){
                    printf("%s",instr);
                }
                ///home/dreamable/a.lql
                //printf(" la instruccion a ejecutar es %s\n",instr);
                //log_debug(logger_debug, instr);

                char* r = exec_instr(instr);
                //log_debug(logger_debug, "exec:obtengo respuesta");
                printf("%s", r);

                
                if(superuser){
                    kernel_call->result = r;
                    pthread_cond_broadcast(&kernel_call->cond);
                    //log_debug(logger_debug, "se le debvuelve al kernel su pedido");
                }

                if(err_trap != 0){
                    //log_error(logger_debug, "EL programa no puede seguir ejecutando.");
                    break;
                }
           
                free(instr);

                /* si el programa se termino de ejecutar
                voy a buscar otro para mantener el nivel de 
                multiprogramacion, si no, lo devuelvo a exec*/
                if(queue_is_empty( program->instr)){
                    //log_debug(logger_debug, "exec:termino programa"); 
                    exec_size--;
                   
                    //log_debug(logg, "exec:me voy a buscar otro programa");
                    lock_queue();
                    updateTasks(exec_queue);
                    
                    break;
                }
                pthread_mutex_lock(&config_lock);
                usleep(config_not->sleep*1000);
                pthread_mutex_unlock(&config_lock);

            }

            if(!queue_is_empty( program->instr) && err_trap==0){
                //log_debug(logger_debug, "exec: termino quantum");
                queue_push(exec_queue, program);


            }else{
                queue_destroy(program->instr);
                free(program);
            }
            
        }

             

        
        
        syscall_availity_status = true;
        //devuelvo el control a consola
        pthread_mutex_unlock(&console->lock);
        lock_queue();
        pthread_cond_broadcast(&console->cond);
        //log_debug(logg, "exec");

        pthread_cond_wait(&queue->cond, &queue->lock);


         /*si mi cola de exec se quedo  vacia quiere decir
        que tambien la del scheduler asi que espero una señal
        ya sea una syscal o que añadan una tarea*/ 
        
        if(!queue_is_empty(syscall_queue->scheduler_queue)){
            //log_debug(logger_debug, "exec:Una syscall libera al procesador ocioso");
            unlock_queue(); //Ya que no voy a actualizar la lista la
           goto EXECUTION;
            
        }else{
            //log_debug(logger_debug, "exec:Una instruccion saco al procesador de modo ocioso");
            updateTasks(exec_queue);
        }
        
        
    }   

    
    
}

//obtiene de la cola del scheduler las proximas tareas
//la cola ya debe estar bloqueada
void updateTasks(t_queue* q){
    
    //log_debug(logg, "exec:actualizando cola de exec");
    while(exec_size != config->multi_script_level && !queue_is_empty(queue->scheduler_queue)){
        
        t_instr_set* new_program = queue_pop(queue->scheduler_queue);
        //log_debug(logg, "nueva instruccion:");
        queue_push(exec_queue, new_program);
        exec_size++;
        //log_debug(logger_debug, "exec:agrego un programa");
        
        
    }
    unlock_queue();
    //log_debug(logger_debug, "exec:fin de acctualizacion");

    
}

void exec_err_abort(){
    err_trap = 1;
}