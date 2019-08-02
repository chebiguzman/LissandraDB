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
#include <commons/collections/dictionary.h>
#include "../pharser.h"
#include "../actions.h"
#include "../console.h"
#include "scheduler.h"
#include <commons/string.h>
#include <commons/collections/queue.h>
#include <signal.h>
#include "metrics_worker.h"

//punto de entrada para el programa y el kernel
 t_log* logger;
 t_log* logger_debug;

char* MEMORY_IP;
int MEMORY_PORT;

int main(int argc, char const *argv[])
{   
    //set up confg

    t_config* config = config_create("config");
    char* LOGPATH = config_get_string_value(config, "LOG_PATH");
    int PORT = config_get_int_value(config, "PORT");
    int q = config_get_int_value(config, "QUANTUM");


    MEMORY_IP = strdup(config_get_string_value(config, "MEMORY_IP"));
    MEMORY_PORT = config_get_int_value(config, "MEMORY_PORT");
    logger = log_create(LOGPATH, "Kernel", 1 , LOG_LEVEL_INFO);

    int console = 0;
      if(argc == 2 && !strcmp(argv[1],"-v")){
      console++;
    }

    logger_debug = log_create("log.debug", "Kernel", console, LOG_LEVEL_DEBUG);
  
    //set up log
    

 
    pthread_cond_t console_cond;
    pthread_mutex_t console_lock;

    pthread_cond_t exec_cond;
    pthread_mutex_t exec_lock;
    pthread_mutex_t print_lock;
    pthread_mutex_init(&console_lock, NULL);
    pthread_cond_init(&console_cond,NULL);
    pthread_mutex_init(&exec_lock, NULL);
    pthread_cond_init(&exec_cond,NULL);
    pthread_mutex_init(&print_lock, NULL);

    t_console_control* control = malloc(sizeof(t_console_control));
    control->lock = console_lock;
    control->cond = console_cond;
    control->print_lock = print_lock;
    control->name = strdup("kernel");
  

   
        


    start_sheduler(logger,logger_debug, control);

    start_kmemory_module(logger,logger_debug, MEMORY_IP, MEMORY_PORT);

   //inicio lectura por consola
    pthread_t tid_console;
    pthread_create(&tid_console, NULL, console_input_wait, control);

    //signal(SIGPIPE, SIG_IGN); //Ignorar error de write
    metrics_start(logger);
    //JOIN THREADS
    //pthread_join(tid,NULL);
    pthread_join(tid_console,NULL);
    //free(LOGPATH);
    config_destroy(config);
    
    //FREE MEMORY
    //free(logger);
    //free(serverInfo);

      return 0;
}

char* exec_in_memory(int memory_fd, char* payload){
   
    char* responce = malloc(3000);
    responce[0] = '\033';
    //strcpy(responce, "");

    if ( memory_fd < 0 ){
      log_error(logger, "No se pudo llevar a cabo la accion.");
      free(responce);
      free(payload);
      exec_err_abort();
      return strdup("");
    }

    void handler(){
      disconect_from_memory(memory_fd);
      log_debug(logger, "proken pipe");
      
    }
    signal(SIGPIPE,handler);

    //ejecutar
    log_debug(logger_debug, "voy a ejecutar en memoria");
    if(write(memory_fd,payload, strlen(payload)+1)){
      read(memory_fd, responce, 3000);
      free(payload);
    log_debug(logger_debug, "memoria responde");

      
      if(responce[0]=='\033'){
        log_error(logger, "Una memoria en uso fue desconectada.");
        disconect_from_memory(memory_fd);
        exec_err_abort();
        free(responce);
        return strdup("");
      }

      return responce;
    }else{
      free(payload);
      free(responce);
      log_error(logger, "No se logo comuniarse con memoria");
      return strdup("NO SE ENCTUENTEA MEMORIA");
    }  
    return strdup("algo sale mal");
}

char* action_select(package_select* select_info){
  log_info(logger_debug, "Se recibio una accion select");
  //get consistency of talble
  t_consistency consistency = get_table_consistency(select_info->table_name);
  log_info(logger_debug, "Se obtiene consistencia accion select es de: %d", consistency);

  //pedir una memoria
  int memoryfd = get_loked_memory(consistency, select_info->table_name);
  log_info(logger_debug, "Se obtiene memoria accion select es:%d", memoryfd);

  char* package = parse_package_select(select_info);
  log_info(logger_debug, "Se parcea accion select:", package);

  clock_t t; 
  t = clock(); 
  char* responce = exec_in_memory(memoryfd, package); 
  t = clock() - t; 
  double time_taken = ((double)t)/CLOCKS_PER_SEC; 
  double* latency = malloc(sizeof(double));
  *latency = time_taken;
  log_info(logger_debug, "Se ejecuta en memoria accion select respuesta:", responce);
  int id = unlock_memory(memoryfd);
  if(id>0) register_select(id, consistency, latency);

  return responce;

}

char* action_run(package_run* run_info){
  log_info(logger_debug, "Se recibio una accion run.");
  char* rt = string_new();
  FILE* fp = fopen(run_info->path, "r");

  if(fp == NULL){
    log_error(logger, "El archivo no existe o no se puede leer.");
    return "";
  }else{
    
    t_queue* instruction_set = queue_create();
    size_t buffer_size = 0;
    char* buffer = NULL;

    while(getline(&buffer, &buffer_size, fp) != -1){
      char* instr_from_file = malloc(strlen(buffer)+1);
      strcpy(instr_from_file, buffer);
      //printf("insturccion a run: %s", instr_from_file);

      queue_push(instruction_set, instr_from_file);

    }
    
    t_instr_set* set = malloc(sizeof(t_instr_set));
    set->instr = instruction_set;
    set->doesPrint = 1;
    schedule(set);

    free(buffer);
    fclose(fp);
  }

    free(run_info->instruction);
    free(run_info->path);
    free(run_info);
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

  free(add_info->instruction);
  free(add_info);
  return strdup("");
}

char* action_insert(package_insert* insert_info){
  log_info(logger_debug, "Se recibio una accion insert");
  t_consistency consistency = get_table_consistency(insert_info->table_name);
  int memoryfd = get_loked_memory(consistency, insert_info->table_name);

  char* package = parse_package_insert(insert_info);

  char* responce = exec_in_memory(memoryfd, package); 
  unlock_memory(memoryfd);
  return responce;
}

char* action_create(package_create* create_info){


  log_info(logger_debug, "Se recibio una accion create");
        
  
  int memoryfd = get_loked_memory(ALL_CONSISTENCY, NULL);
  
  char* tbl_name = strdup(create_info->table_name);
  t_consistency c = create_info->consistency;
  char* package = parse_package_create(create_info);
  char* responce = exec_in_memory(memoryfd, package);
  kmemoy_add_table(tbl_name, c);
  free(tbl_name);
  unlock_memory(memoryfd);
  return responce;
}

char* action_describe(package_describe* describe_info){
  log_info(logger_debug, "Se recibio una accion describe");
  int memoryfd = get_loked_memory(ALL_CONSISTENCY, NULL);
<<<<<<< HEAD
<<<<<<< HEAD

  char* table_name = NULL;
  if(describe_info->table_name!=NULL) table_name = strdup(describe_info->table_name);
=======
>>>>>>> 709054a6e23cdd3936903a4425098f41ef5a3763
=======
>>>>>>> 8b208718164a1c0aacd23c90187b2a621e4b4c2e
  char* package = parse_package_describe(describe_info);

  char* responce = exec_in_memory(memoryfd, package);
  if(strlen(responce)>20){
    char** buffer = string_split(responce, ";");
    t_dictionary* tables_dic = dictionary_create();

    while(*buffer){
      char** lines = string_split(*buffer, "\n");
      t_dictionary* dic = dictionary_create();

      void add_cofiguration(char *line) {
        if (!string_starts_with(line, "#")) {
          char** keyAndValue = string_n_split(line, 2, "=");

          dictionary_put(dic,  keyAndValue[0], keyAndValue[1]);
          free(keyAndValue[0]);
          free(keyAndValue);
        }
    }
      string_iterate_lines(lines, add_cofiguration);
      string_iterate_lines(lines, (void*) free);

      char* name = dictionary_get(dic, "NOMBRE");
      char* cons = dictionary_get(dic, "CONSISTENCY");
         
      if(name!=NULL && cons !=NULL){
        string_to_upper(name);
        int* constistency = malloc(sizeof(int));

        *constistency = atoi(cons);

        dictionary_put(tables_dic, name, constistency);
      
<<<<<<< HEAD
<<<<<<< HEAD
        if(table_name!=0){
          kmemory_add_table(name,dictionary_get(dic, "CONSISTENCY") );
          free(table_name);
=======
        if(describe_info->table_name!=NULL){
          kmemory_add_table(name,dictionary_get(dic, "CONSISTENCY") );
>>>>>>> 709054a6e23cdd3936903a4425098f41ef5a3763
=======
        if(describe_info->table_name!=NULL){
          kmemory_add_table(name,dictionary_get(dic, "CONSISTENCY") );
>>>>>>> 8b208718164a1c0aacd23c90187b2a621e4b4c2e
        }
      }
      
      
      free(lines);
      free(dic);

      buffer++;
    }
<<<<<<< HEAD
<<<<<<< HEAD
      if(table_name==NULL ){
=======
      if(describe_info->table_name == NULL){
        printf("describe generala %s\n",describe_info->table_name);
>>>>>>> 709054a6e23cdd3936903a4425098f41ef5a3763
=======
      if(describe_info->table_name == NULL){
        printf("describe generala %s\n",describe_info->table_name);
>>>>>>> 8b208718164a1c0aacd23c90187b2a621e4b4c2e
        kmemory_set_active_tables(tables_dic);

      }
  }

 
  unlock_memory(memoryfd);
  return responce;
}

char* action_drop(package_drop* drop_info){
  log_info(logger_debug, "Se recibio una accion drop");
  int memoryfd = get_loked_memory(ALL_CONSISTENCY, NULL);
  kmemory_drop_table(drop_info->table_name);
  char* package = parse_package_drop(drop_info);
  char* responce = exec_in_memory(memoryfd, package);
  unlock_memory(memoryfd);
  return responce;
}

char* action_journal(package_journal* journal_info){
  log_info(logger_debug, "Se recibio una accion select");
  int memoryfd = get_loked_memory(ALL_CONSISTENCY, NULL);
  char* package = parse_package_journal(journal_info);
  char* responce = exec_in_memory(memoryfd, package);
  unlock_memory(memoryfd);
  return responce;
}

char* action_metrics(package_metrics* metrics_info){
  log_info(logger_debug, "Se recibio una accion metrics");
  free(metrics_info->instruction);
  free(metrics_info);
  return get_metrics();
}

char* action_intern__status(){

  log_info(logger_debug, "Se recibio una accion status");

  int memfd = get_loked_main_memory();
  log_info(logger_debug, "obtube memoria principal");
  
  if(memfd<0){
    //la memoria principal fue desconectada
    log_debug(logger_debug, "No existe memoria principal para hacer gossiping");
    log_debug(logger_debug, "Se intenta comunicar con la memoria por default");

    /* t_config* config = config_create("config");

    char* main_ip = strdup(config_get_string_value(config, "QUANTUM"));
    int main_port = config_get_int_value(config, "MEMORY_PORT");
    config_destroy(config);*/

    //printf("se intenta consectar con la principal ip:%s", main_ip);

    check_for_new_memory(MEMORY_IP ,MEMORY_PORT, 0);
    return strdup("");
  }

  log_info(logger_debug, "x ejecutar en mem");

  char* responce = exec_in_memory(memfd, strdup("MEMORY"));
  log_info(logger_debug, "ejecute, la resupuesta fue: %s", responce);

  unlock_memory(memfd);
  if(responce == NULL || strlen(responce)==0 ){
    if(responce!=NULL) free(responce);
    return strdup("");
  }

  
  char** memories = string_split(responce, "|");
  update_memory_finder_service_time(atoi(memories[1]));
  set_main_memory_id(atoi(memories[0]));
  free(memories[0]);
  free(memories[1]);
    int i = 2;

    while(memories[i]!=NULL){
      log_debug(logger_debug, "%s", memories[i]);
      if(strlen(memories[i])<14) break;
      char** info = string_split(memories[i], ",");
      int id = atoi(info[0]);
      char* ip = info[1];
      int port = atoi(info[2]);
      check_for_new_memory(ip ,port, id);

      free(ip);
      free(info[0]);
      free(info[2]);
      free(info);
      free(memories[i]);
      i++;
    }
    free(memories);
    free(responce);
    return strdup("");
}
char* action_gossip(char* buffer){
  return strdup("");
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
