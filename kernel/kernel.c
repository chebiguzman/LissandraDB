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
#include <commons/collections/queue.h>
#include <signal.h>


//punto de entrada para el programa y el kernel
t_log* logger;
int memoryfd;
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

    //set up server
   /* pthread_t tid;
    server_info* serverInfo = malloc(sizeof(server_info));
    memset(serverInfo, 0, sizeof( server_info));    
    serverInfo->logger = logger;
    serverInfo->portNumber = PORT;
    pthread_create(&tid, NULL, create_server, (void*) serverInfo);*/
    
    

    
    //set up client
    
    pthread_cond_t console_cond;
    pthread_mutex_t console_lock;
    pthread_mutex_init(&console_lock, NULL);
    pthread_cond_init(&console_cond,NULL);
    t_console_control* control = malloc(sizeof(t_console_control));
    control->lock = console_lock;
    control->cond = console_cond;
    control->name = strdup("kernel");

    start_sheduler(logger,control);
    start_kmemory_module(logger, MEMORY_IP, MEMORY_PORT);
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

char* action_select(package_select* select_info){
  log_info(logger, "Se recibio una accion select");
  char* responce = malloc(3000);
  strcpy(responce, "");

  //get consistency of talble
  t_consistency consistency = get_table_consistency(select_info->table_name);
  //pedir una memoria
  int memoryfd = get_loked_memory(consistency);
  printf("\nla fd de mem es: %d", memoryfd);
  if ( memoryfd < 0 ){
    log_error(logger, "No se pudo llevar a cabo la accion select");
    return "";
  }

  //ejecutar
  /*char* package = parse_package_select(select_info);
  if(write(memoryfd,package, strlen(package)+1)){
    read(memoryfd, responce, 3000);
    
  }else{
    log_error(logger, "No se logo comuniarse con memoria");
  }
  */
  //debolver memoria
  unlock_memory(memoryfd);
  //dar resou
 
  return responce;

}


char* action_run(package_run* run_info){
  log_info(logger, "Se recibio una accion run");
  char* rt = string_new();
  FILE* fp = fopen(run_info->path, "r");

  if(fp == NULL){
    log_error(logger, "El archivo no existe o no se puede leer");
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
  log_info(logger, "Se recibio una accion add");
  return "";
}


void action_insert(package_insert* insert_info){
  log_info(logger, "Se recibio una accion insert");
}

void action_create(package_create* create_info){
  log_info(logger, "Se recibio una accion create");
}

void action_describe(package_describe* describe_info){
  log_info(logger, "Se recibio una accion describe");
}

void action_drop(package_drop* drop_info){
  log_info(logger, "Se recibio una accion drop");
}

void action_journal(package_journal* journal_info){
  log_info(logger, "Se recibio una accion select");
}

void action_metrics(package_metrics* metrics_info){
  log_info(logger, "Se recibio una accion metrics");
}

//Crea un t_instr con un string
char* parse_input(char* input){
  t_instr_set* set = malloc(sizeof(t_instr_set));
  t_queue* q = queue_create();
  char* buff = strdup(input);
  queue_push(q, buff);
  set->doesPrint = 1;
  set->instr = q;
  schedule(set);
  return "";
}
