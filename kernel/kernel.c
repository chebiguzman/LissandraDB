#include <stdlib.h>
#include <commons/log.h>
#include <pthread.h>
#include <string.h>
#include "../server.h"
#include <commons/config.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>

#include "../pharser.h"
#include "../actions.h"
#include "../console.h"
#include "scheduler.h"
#include <commons/string.h>
#include <commons/collections/queue.h>
#include <signal.h>


//punto de entrada para el programa y el kernel
t_log* logger;
t_log* logger_debug;
int main(int argc, char const *argv[])
{   
    //set up confg
    t_config* config = config_create("config");
    char* LOGPATH = config_get_string_value(config, "LOG_PATH");
    int PORT = config_get_int_value(config, "PORT");
    int q = config_get_int_value(config, "QUANTUM");

    char* MEMORY_IP = config_get_string_value(config, "MEMORY_IP");
    int MEMORY_PORT = config_get_int_value(config, "MEMORY_PORT");

   
    //set up log
    logger = log_create(LOGPATH, "Kernel", 1 , LOG_LEVEL_DEBUG);
    int console = 0;

    if(argc == 2 && !strcmp(argv[1],"-v")){
      console++;
    }
    logger_debug = log_create(LOGPATH, "Kernel", 0, LOG_LEVEL_DEBUG);
  
    pthread_cond_t console_cond;
    pthread_mutex_t console_lock;
    pthread_mutex_init(&console_lock, NULL);
    pthread_cond_init(&console_cond,NULL);
    t_console_control* control = malloc(sizeof(t_console_control));
    control->lock = console_lock;
    control->cond = console_cond;
    control->name = strdup("kernel");

    start_sheduler(logger,logger_debug, control);

    start_kmemory_module(logger,logger_debug, MEMORY_IP, MEMORY_PORT);
        

   //inicio lectura por consola
    pthread_t tid_console;
    pthread_create(&tid_console, NULL, console_input_wait, control);
    
    signal(SIGPIPE, SIG_IGN); //Ignorar error de write

    //JOIN THREADS
    //pthread_join(tid,NULL);
    pthread_join(tid_console,NULL);
    
    //FREE MEMORY
    free(LOGPATH);
    free(logger);
    //free(serverInfo);
    config_destroy(config);

      return 0;
}

char* exec_in_memory(int memory_fd, char* payload){
    char* responce = malloc(3000);
    strcpy(responce, "");
    
    if ( memory_fd < 0 ){
      log_error(logger, "No se pudo llevar a cabo la accion.");
      return "";
    }

    //ejecutar
    if(write(memory_fd,payload, strlen(payload)+1)){
      read(memory_fd, responce, 3000);
      return responce;
    }else{
      log_error(logger, "No se logo comuniarse con memoria");
      return "NO SE ENCTUENTEA MEMORIA";
    }  
    return "algo sale mal";
}

char* action_select(package_select* select_info){
  log_info(logger_debug, "Se recibio una accion select");
  //get consistency of talble
  t_consistency consistency = get_table_consistency(select_info->table_name);

  //pedir una memoria
  int memoryfd = get_loked_memory(consistency, select_info->table_name);
  char* package = parse_package_select(select_info);

  char* responce = exec_in_memory(memoryfd, package); 
 
  unlock_memory(memoryfd);
  return responce;

}

char* action_run(package_run* run_info){
  log_info(logger_debug, "Se recibio una accion run.");
  char* rt = string_new();
  FILE* fp = fopen(run_info->path, "r");

  if(fp == NULL){
    log_error(logger, "El archivo no existe o no se puede leer.");
  }else{
    
    t_queue* instruction_set = queue_create();
    size_t buffer_size = 0;
    char* buffer = NULL;

    while(getline(&buffer, &buffer_size, fp) != -1){
      char* instr_from_file = malloc(buffer_size);
      memcpy(instr_from_file, buffer, buffer_size);
      queue_push(instruction_set, instr_from_file);

    }
    
    t_instr_set* set = malloc(sizeof(t_instr_set));
    set->instr = instruction_set;
    set->doesPrint = 1;
    schedule(set);

    free(buffer);
    fclose(fp);
  }

  
    return rt;
}

char* action_add(package_add* add_info){
  log_info(logger_debug, "Se recibio una accion add");
  if(add_info->consistency == S_CONSISTENCY){
    add_memory_to_sc(add_info->id);
  }else if(add_info->consistency == ANY_CONSISTENCY){
    add_memory_to_any(add_info->id);
  }else if( add_info->consistency == H_CONSISTENCY){
    add_memory_to_hc(add_info->id);
  }
  return "";
}

char* action_insert(package_insert* insert_info){
  log_info(logger_debug, "Se recibio una accion insert");
  /*t_consistency consistency = get_table_consistency(insert_info->table_name);
  int memoryfd = get_loked_memory(consistency, insert_info->table_name);
  char* package = parse_package_insert(select_info);

  char* responce = exec_in_memory(memoryfd, package); 
 
  unlock_memory(memoryfd);*/
  return "";
}

char* action_create(package_create* create_info){
  log_info(logger_debug, "Se recibio una accion create");
  int memoryfd = get_loked_memory(ALL_CONSISTENCY, NULL);
  return "";
}

char* action_describe(package_describe* describe_info){
  log_info(logger_debug, "Se recibio una accion describe");
  int memoryfd = get_loked_memory(ALL_CONSISTENCY, NULL);
  return "";
}

char* action_drop(package_drop* drop_info){
  log_info(logger_debug, "Se recibio una accion drop");
  int memoryfd = get_loked_memory(ALL_CONSISTENCY, NULL);
  return "";
}

char* action_journal(package_journal* journal_info){
  log_info(logger_debug, "Se recibio una accion select");
  int memoryfd = get_loked_memory(ALL_CONSISTENCY, NULL);
  return "";
}

char* action_metrics(package_metrics* metrics_info){
  log_info(logger_debug, "Se recibio una accion metrics");
  
  return "";
}

char* action_intern_memory_status(){
  log_info(logger_debug, "Se recibio una accion status");

  int memfd = get_loked_main_memory();
  
  if(memfd<0){
    return "";
  }

  char* responce = exec_in_memory(memfd, "MEMORY");
  unlock_memory(memfd);
  if(responce == ""){
    return "";
  }
  char** memories = string_split(responce, "|");
  update_memory_finder_service_time(atoi(memories[0]));
    int i = 1;

    while(memories[i]!=NULL){
      char** info = string_split(memories[i], ",");
      int id = atoi(info[0]);
      char* ip = info[1];
      int port = atoi(info[2]);

      check_for_new_memory(ip ,port, id);
      i++;
    }
    return "";
}

//Crea un t_instr con un string
char* parse_input(char* input){
  
  t_instr_set* set = malloc(sizeof(t_instr_set));
  t_queue* q = queue_create();
  char* buff = strdup(input);
  queue_push(q, buff);
  set->doesPrint = 0;
  set->instr = q;
  schedule(set);
  return "";
}
