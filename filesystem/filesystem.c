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
#include "../console.h"
#include "engine.h"
#include <dirent.h>
#include <errno.h>
#include "memtable.h"
#include <sys/stat.h>
#include <fcntl.h>
#include "filesystem.h"
#include <pthread.h>
//punto de entrada para el programa y el kernel
t_log* logger;

int main(int argc, char const *argv[]){
    
    //las estructuras se van al .h para que quede mas limpio
    //set up confg
    t_config* config = config_create("config");
    char* LOGPATH = config_get_string_value(config, "LOG_PATH");
    MNT_POINT = config_get_string_value(config, "PUNTO_MONTAJE");
    int PORT = config_get_int_value(config, "PORT");

    //set up log
    logger = log_create(LOGPATH, "Filesystem", 1, LOG_LEVEL_INFO);

    engine_start(logger);


    //set up dump
    int TIEMPO_DUMP = config_get_int_value(config, "TIEMPO_DUMP");
    pthread_t tid_dump;
    pthread_create(&tid_dump, NULL, dump_cron, (void*) TIEMPO_DUMP);
        
    //set up server
    server_info* serverInfo = malloc(sizeof(server_info));
    serverInfo->logger = logger;
    serverInfo->portNumber = PORT;
    pthread_t tid;
    pthread_create(&tid, NULL, create_server, (void*) serverInfo);
 
    /*fs_structure_info->logger = logger;
    pthread_t tid_fs_structure;
    no hay necesidad de un thread aca
    //pthread_create(&tid_fs_structure, NULL, setup_fs, (void*) fs_structure_info);*/

    //inicio lectura por consola
    pthread_t tid_console;
    pthread_create(&tid_console, NULL, console_input, "fileSystem");

    //JOIN THREADS
    pthread_join(tid,NULL);
    
    //FREE MEMORY
    free(LOGPATH);
    free(logger);
    free(serverInfo);
    //free(fs_structure_info);
    config_destroy(config);
    

    return 0;
}

//IMPLEMENTACION DE ACCIONES (Devolver error fuera del subconjunto)

char* action_select(package_select* select_info){
  log_info(logger, "Se recibio una accion select");


  if(!does_table_exist(select_info->table_name)){
    return "La tabla solicitada no existe.\n";
  }

  t_table_metadata* meta = get_table_metadata(select_info->table_name);

  //nro particion
  int table_partition_number = select_info->key % meta->partition_number ;

  t_table_partiton* partition = get_table_partition(select_info->table_name, table_partition_number);

  
  int block_amount = 0;
  char* first_block = partition->blocks[0];
  while(*partition->blocks){
    block_amount++;
    *partition->blocks++;
  }
  *partition->blocks = first_block;

   if(block_amount==0)return "Key invalida\n";
  
  pthread_t buscadores[block_amount];
  regg regruta[block_amount];

  int i = 0;
  while(i<block_amount){
    regruta[i].line=malloc(100);
    strcpy(regruta[i].line,"MountTest/");
    strcat(regruta[i].line,"Bloques/");
    strcat(regruta[i].line,partition->blocks[i]);

    strcat(regruta[i].line,".bin");
    
    log_info(logger,regruta[i].line);
    i++;
  }


  pthread_mutex_t lock;
  pthread_cond_t cond;
  pthread_mutex_init(&lock, NULL);
  pthread_cond_init(&cond, NULL);

  int whilethread=0;
  argumentosthread* parametros [block_amount];
  int* number_of_threads = malloc(sizeof(int));
  *number_of_threads = block_amount;

  while(whilethread<block_amount){
    argumentosthread* args = malloc(sizeof(argumentosthread));
    args->bolean=0;
    args->ruta = strdup(regruta[whilethread].line);
    args->key=select_info->key;
    args->cond = &cond;
    args->lock = lock;
    args->number_of_running_threads = number_of_threads;
    parametros[whilethread] = args;
    pthread_create(&buscadores[whilethread],NULL,buscador,args);
    pthread_detach(buscadores[whilethread]);
    whilethread++;
  }

  pthread_mutex_lock(&lock);
  pthread_cond_wait(&cond, &lock);
  int whileparametro=0;
  while(whileparametro<block_amount){
    if(parametros[whileparametro]->bolean){
      char* r = malloc( strlen(parametros[whileparametro]->value) + 2);
      strcpy(r, parametros[whileparametro]->value);
      strcat(r, "\n");
      return r;
    }
    whileparametro++;
  }

  pthread_mutex_destroy(&lock);
  pthread_cond_destroy(&cond);

  return "Key invalida\n";
  //falta atender los memory leaks, en especial los de los thread.

}

char* action_insert(package_insert* insert_info){

  if(!does_table_exist(insert_info->table_name)){
    log_error(logger, "No se puede completar el describe.");
    return "La tabla no existe.\n";
  }
  char* table_name = insert_info->table_name;
  char* table_path = malloc(sizeof(table_name)+sizeof(MNT_POINT)+sizeof("Tables/"));
  table_path[0] = '\0';
  
  strcat(table_path ,MNT_POINT);
  strcat(table_path ,"Tables/");
  strcat(table_path ,table_name);

  insert_to_memtable(insert_info);
 
  log_debug(logger, "Se inserto el valor en la memtable");
  free(table_path);

  return "";
  
}

char* action_create(package_create* create_info){
  log_info(logger, "Se recibio una accion create");
  
  if(does_table_exist(create_info->table_name)){
    char* err = "Fallo la creacion de una tabla.\n";
    log_error(logger, err);
    return "La tabla ya existe";
  }

  enginet_create_table(create_info->table_name, create_info->consistency, create_info->partition_number, create_info->compactation_time);
  
  return "";
}

char* action_describe(package_describe* describe_info){
  log_info(logger, "Se recibio una accion describe");

  //distingo si cargaron o no una tabla a describir

  if (describe_info->table_name != NULL) {
    
    if(!does_table_exist(describe_info->table_name)){
      log_error(logger, "No se puede completar el describe.");
      return "La tabla no existe.\n";
    }

    char* meta = get_table_metadata_as_string(describe_info->table_name);
    char* result = malloc( strlen(meta) + strlen(describe_info->table_name) +2);
    strcpy(result, describe_info->table_name);
    strcat(result, "\n");
    strcat(result, meta);
    strcat(result, "\n");
    free(meta);
    return result;

  }

  char* result = get_all_tables_metadata_as_string();
  
  return result;
}

char* action_drop(package_drop* drop_info){

  if(!does_table_exist(drop_info->table_name)){
    return "La tabla solicitada no existe.\n";
  }
  engine_drop_table(drop_info->table_name);

  return "";
}

char* action_journal(package_journal* journal_info){
  log_info(logger, "Se recibio una accion select");
  
  return "";
}

char* action_add(package_add* add_info){
  log_info(logger, "Se recibio una accion select");
  return "";
}

char* action_run(package_run* run_info){
  log_info(logger, "Se recibio una accion run");
  dump_memtable();
  return "";
}

char* action_metrics(package_metrics* metrics_info){
  log_info(logger, "Se recibio una accion metrics");
  return "";
}

//ACA VA A HABER QUE CREAR THREADS DE EJECUCION
char* parse_input(char* input){
  return exec_instr(input);
}

char* action_intern_memory_status(){ return "";};

char *strdups(const char *src) {
    char *dst = malloc(strlen (src) + 1);  
    if (dst == NULL) return NULL;     
    strcpy(dst, src);                     
    return dst; 
}


void vaciarvector(char* puntero){
  for(int i=0;i<100;i++){
  puntero[i]='\0';
}
return;
}

void* buscador(void* args){
  argumentosthread* parametros;
  parametros= (argumentosthread*) args;
  FILE* bloque=NULL;

  void kill_thread(){
    pthread_mutex_lock(&parametros->lock);
    parametros->number_of_running_threads--;
    int amount = *parametros->number_of_running_threads;
    pthread_mutex_unlock(&parametros->lock);
    if(amount==0) pthread_cond_broadcast(parametros->cond);
  }

  bloque=fopen(parametros->ruta,"r+");
  if(bloque==NULL){
    log_error(logger,"El sistema de bloques de archivos presenta una inconcistencia en el bloque:");
    log_error(logger,parametros->ruta);
    log_error(logger, "el archivo no existe.");
    kill_thread();
  
    return NULL;
  }

  char buffer[100];
  parametros->retorno = strdup("");
  while(!feof(bloque)){
    fgets(buffer,100,bloque);
    parametros->row= strdup(buffer);
    //devuelve key

    cortador(buffer,parametros->retorno);
    if(parametros->key==atoi(parametros->retorno)){
      obtengovalue(parametros->row,parametros->value);
      parametros->bolean=1;
      pthread_cond_broadcast(parametros->cond);
      fclose(bloque);
      return NULL;
    }
    parametros->retorno = strdup("");
  }
  kill_thread();

  fclose(bloque);
  return NULL;
}

void cortador(char* cortado, char* auxkey){
  int i=0;
  int j=0;
  while(cortado[i]!=';' && cortado[i]!='\n'){
      i++;
  }
  i++;

  while(cortado[i]!=';' && cortado[i]!='\n'){
     auxkey[j]=cortado[i];
     i++;
     j++;
  }
  return;
}

void obtengovalue(char* row, char* value){
  int largo=strlen(row);
  int i= 0;
  int j= 0;
  int veces=0;
  while(row[i]!=';' && row[i]!='\n'){
      i++;
  }
  i++;
  while(row[i]!=';' && row[i]!='\n'){
      i++;
  }
  i++;
  int colocar= largo - i;
  while(i<largo){
      value[j]=row[i];
      i++;
      j++;
  }
  value[colocar]='\0';
  return;
}

void* dump_cron(void* TIEMPO_DUMP) {
  while(1) {
    sleep((int) TIEMPO_DUMP / 1000);
    dump_memtable();
  }
}
