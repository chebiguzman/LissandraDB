#include <stdlib.h>
#include <commons/log.h>
#include <pthread.h>
#include <string.h>
#include "../server.h"
#include <commons/config.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/inotify.h>
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
#define BLOCK_SIZE_DEFAULT 128
//punto de entrada para el programa y el kernel
t_log* logger;
int VALUE_SIZE;
int main(int argc, char const *argv[]){
   
    //las estructuras se van al .h para que quede mas limpio
    //set up confg
    t_config* config = config_create("config");
    char* LOGPATH = config_get_string_value(config, "LOG_PATH");
    MNT_POINT = config_get_string_value(config, "PUNTO_MONTAJE");
    VALUE_SIZE = config_get_int_value(config, "TAMAÑO_VALUE");
    int PORT = config_get_int_value(config, "PORT");
 
    //set up log
    logger = log_create(LOGPATH, "Filesystem", 1, LOG_LEVEL_INFO);
 
    engine_start(logger);
 
 
    //set up dump
    int dump_time_buffer = config_get_int_value(config, "TIEMPO_DUMP");
    int *TIEMPO_DUMP = &dump_time_buffer;
 
     pthread_t tid_dump;
     pthread_create(&tid_dump, NULL, dump_cron, (void*) TIEMPO_DUMP);
   
   
    //set up server
    server_info* serverInfo = malloc(sizeof(server_info));
    serverInfo->logger = logger;
    serverInfo->portNumber = PORT;
    pthread_t tid;
    pthread_create(&tid, NULL, create_server, (void*) serverInfo);
   
 
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
 
  usleep(get_retardo_time());
 
  if(!does_table_exist(select_info->table_name)){
    free(parse_package_select(select_info));
    return strdup("La tabla solicitada no existe.\n");
  }
 
  if(is_data_on_memtable(select_info->table_name, select_info->key)){
      char* r = malloc(strlen(get_value_from_memtable(select_info->table_name, select_info->key) + 2));
      strcpy(r, get_value_from_memtable(select_info->table_name, select_info->key));
      strcat(r, "\n");
      free(parse_package_select(select_info));
 
    return r;
  }
 
  t_table_metadata* meta = get_table_metadata(select_info->table_name);
 
  //nro particion
 
  int table_partition_number = select_info->key % meta->partition_number ;
 
  t_table_partiton* partition = get_table_partition(select_info->table_name, table_partition_number);
 
  free(meta);
 
  int block_amount = 0;
  void* first_block = partition->blocks;
  while(*partition->blocks){
    block_amount++;
    partition->blocks++;
  }
  partition->blocks = first_block;
 
  if(block_amount==0)return strdup("Key invalida\n");
 
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
 
  free(partition);
 
  pthread_mutex_lock(&lock);
  pthread_cond_wait(&cond, &lock);
  int whileparametro=0;
  while(whileparametro<block_amount){
    if(parametros[whileparametro]->bolean){
      char* r = malloc( strlen(parametros[whileparametro]->value) + 2);
      strcpy(r, parametros[whileparametro]->value);
     
 
      //strcat(r, "\n");
      return r;
    }
    whileparametro++;
  }
 
  free(parse_package_select(select_info));
 
  return strdup("Key invalida\n");
  //falta atender los memory leaks, en especial los de los thread.
 
}
 
int min(int a, int b){
  if(a>b){
    return b;
  }
  return a;
}
 
char* action_insert(package_insert* insert_info){
 
  usleep(get_retardo_time());
 
  if(!does_table_exist(insert_info->table_name)){
    log_error(logger, "No se puede completar el describe.");
    free(parse_package_insert(insert_info));
    return strdup("La tabla no existe.\n");
  }
  char* table_name = insert_info->table_name;
  char* table_path = malloc(strlen(table_name)+strlen(MNT_POINT)+strlen("Tables/")+1);
  table_path[0] = '\0';
 
  strcat(table_path ,MNT_POINT);
  strcat(table_path ,"Tables/");
  strcat(table_path ,table_name);
 
  char* sliced_value = malloc(VALUE_SIZE+2);
  int len = min(VALUE_SIZE, strlen(insert_info->value));
  memcpy(sliced_value, insert_info->value, len);
  strcpy(sliced_value+len, "\0");
  printf("%s\n", sliced_value);
  free(insert_info->value);
  insert_info->value = sliced_value;
 
  insert_to_memtable(insert_info);
 
  printf("Se agrego en la memtable\n");
 
  log_debug(logger, "Se inserto el valor en la memtable");
  free(table_path);
  //free(parse_package_insert(insert_info));  
  return strdup("");
 
}
 
char* action_create(package_create* create_info){
  log_info(logger, "Se recibio una accion create");
  usleep(get_retardo_time());
 
  if(does_table_exist(create_info->table_name)){
    char* err = "Fallo la creacion de una tabla.\n";
    log_error(logger, err);
    free(parse_package_create(create_info));
    return strdup("La tabla ya existe\n");
  }
 
  enginet_create_table(create_info->table_name, create_info->consistency, create_info->partition_number, create_info->compactation_time);
 
  return strdup("");
}
 
char* action_describe(package_describe* describe_info){
  log_info(logger, "Se recibio una accion describe");
  usleep(get_retardo_time());
 
  //distingo si cargaron o no una tabla a describir
 
  if (describe_info->table_name != NULL) {
   
    if(!does_table_exist(describe_info->table_name)){
      log_error(logger, "No se puede completar el describe.");
      free(parse_package_describe(describe_info));
      return strdup("La tabla no existe.\n");
    }
    //string_to_upper(describe_info->table_name);
    char* meta = get_table_metadata_as_string(describe_info->table_name);
 
    char* result = malloc( strlen(meta) + strlen(describe_info->table_name) + strlen("NAME=") +8);
    strcpy(result,"NAME=");
    strcat(result, describe_info->table_name);
    strcat(result, "\n");
    strcat(result, meta);
    strcat(result, ";\n\n");
    free(meta);
    free(parse_package_describe(describe_info));
 
    return result;
 
  }
 
  char* result = get_all_tables_metadata_as_string();
 
  return result;
}
 
char* action_drop(package_drop* drop_info){
 
  if(!does_table_exist(drop_info->table_name)){
    free(parse_package_drop(drop_info));
    return strdup("La tabla solicitada no existe.\n");
  }
  engine_drop_table(drop_info->table_name);
 
  return strdup("");
}
 
char* action_journal(package_journal* journal_info){
  free(parse_package_journal(journal_info));
  log_info(logger,"wat?");
   engine_compactate(strdup("A"));
  return strdup("No es una instruccion valida\n");
}
 
char* action_add(package_add* add_info){
  free(add_info->instruction);
  free(add_info);
  return strdup("No es una instruccion valida\n");
}
 
char* action_run(package_run* run_info){
  free(run_info->instruction);
  free(run_info->path);
  free(run_info);
  return strdup("No es una instruccion valida\n");
}
 
char* action_metrics(package_metrics* metrics_info){
  free(metrics_info->instruction);
  free(metrics_info);
  return strdup("No es una instruccion valida\n");
}
 
//ACA VA A HABER QUE CREAR THREADS DE EJECUCION
char* parse_input(char* input){
  return exec_instr(input);
}
 
char* action_intern__status(){
  return string_itoa(VALUE_SIZE);
};
 
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
  fflush(stdout);
  while(1) {
    sleep( get_dump_time() / 1000);
    dump_memtable();
  }
}
 
 
void particiontemporal(char* temporal,char* tabla){
 
  char* ruta=malloc(100);
  strcpy(ruta,"MountTest/Tables/") ;
  strcat(ruta,tabla);
  strcat(ruta,"/");
  strcat(ruta,temporal);
 
  log_info(logger,ruta);
  int numparticion=partition_num(temporal);
  char* numparticion_aux=string_itoa(numparticion);
  log_info(logger,numparticion_aux);
 
  t_table_partiton* particion= get_table_partition2(tabla, numparticion);
 
  int block_amount = 0;
  char* first_block = particion->blocks[0];
  log_info(logger,"antes de la iteracion");
  while(*particion->blocks){
    block_amount++;
    *particion->blocks++;
  }
  *particion->blocks = first_block;  
  regg regruta[block_amount];
  regg temp_rows[40];
  int reg_amount;
  int i = 0;
  int block_number;
  char* paloggear;
  while(i<block_amount){
    regruta[i].line=malloc(100);
    strcpy(regruta[i].line,"MountTest/");
    strcat(regruta[i].line,"Bloques/");
    strcat(regruta[i].line,particion->blocks[i]);
    block_number=atoi(particion->blocks[i]);
    strcat(regruta[i].line,".bin");
    log_info(logger,regruta[i].line);
 
    reg_amount= get_all_rows(regruta[i].line,temp_rows,block_number);
    paloggear=string_itoa(reg_amount);
    log_info(logger,paloggear);
 
    reubicar_rows(temp_rows,tabla,reg_amount);
    i++;
  }
  free(ruta);
  return ;
}
 
int partition_num(char* numero){
  char** name_parts = string_split(numero, ".");
  printf("la primera parte del nombre es:%s\n",name_parts[0]);
  int retorno = atoi(name_parts[0]);
  free(name_parts[0]);
  free(name_parts[1]);
  free(name_parts);
  return retorno;
}
 
int get_all_rows(char* ruta,regg* rows,int block_number){
  log_info(logger,ruta);
  FILE* bloque=fopen(ruta,"r");
  int registro=0;
  while(!feof(bloque)){
    rows[registro].line=malloc(100);// cambiar max tam;
    fgets(rows[registro].line,100,bloque);
    registro++;
  }
  char* hola=string_itoa(block_number);
  log_info(logger,"el bloque a liberar es");
  log_info(logger,hola);
  fclose(bloque);
  bloque=fopen(ruta,"w");
  fclose(bloque);
  set_block_as_free(block_number);
  return registro;
}
 
int get_row_key(char* row ){
  //printf("obterner la key de la row:%s\n", row);
  if(row == NULL) return -1;
  char** parts = string_split(row, ";");
  if(parts == NULL) return -1;
  if(parts[0] == NULL) return -1;
  if(parts[1] == NULL) return -1;
  int r = atoi(parts[1]);
 
  void free_s(char * s){
    free(s);
  }
 
  string_iterate_lines(parts, free_s);
 
 
  free(parts);
  return r;
}
 
void reubicar_rows(regg* row_list,char* tabla,int reg_amount){
  int cantidad_maxima = 1;//max_row_amount();
  t_table_metadata* metadata= get_table_metadata(tabla);
  char* auxkey = malloc(30);
  int q=0;
  reg_amount--;
 
  while(q<reg_amount){
    char* aux_reg_amount=string_itoa(reg_amount);
    log_info(logger,"recibo esta cantidad de registros:");
    log_info(logger,aux_reg_amount);
    printf("la row es:%s\n", row_list[q].line);
    printf("q es %d\n", q);
 
    int key= get_row_key(row_list[q].line);
    printf("key es %d\n", key);
 
    int part=key % metadata->partition_number;
    log_info(logger,tabla);
    t_table_partiton* currentpartition=get_table_partition(tabla,part);
   
    int block_amount = 0;
    log_info(logger,"antes del while");
    void* first_block = currentpartition->blocks;
    while(*currentpartition->blocks){
      log_info(logger,"entre al while");
      printf("los bloques son: %s",*currentpartition->blocks);
      block_amount++;
      currentpartition->blocks++;
    }
    currentpartition->blocks = first_block;
   
    log_info(logger,"antes del if");
     if(block_amount==0){
       log_info(logger,"adentro del if");
      new_block(row_list[q].line,tabla,part);
      q++;
    }
    else{
    pthread_t buscadores[block_amount];
    regg regruta[block_amount];
    log_info(logger,"cantidad de bloques:");
    char* amount_aux=string_itoa(block_amount);
    log_info(logger,amount_aux);
    int i = 0;
    log_info(logger,"bloques:");
    while(i<block_amount){
      regruta[i].line=malloc(100);
      strcpy(regruta[i].line,"MountTest/");
      strcat(regruta[i].line,"Bloques/");
      strcat(regruta[i].line,*currentpartition->blocks);
      strcat(regruta[i].line,".bin");
      log_info(logger,regruta[i].line);
      log_info(logger,"una vuelta de armado de rutas");
      i++;
      *currentpartition->blocks++;
    }
    pthread_mutex_t lock;
    pthread_cond_t cond;
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);
    log_info(logger,"semaforos levantados");
    int whilethread=0;
    argumentosthread_compactacion* parametros [block_amount];
    int* number_of_threads = malloc(sizeof(int));
    *number_of_threads = block_amount;
    log_info(logger,"mallock hecho");
 
    while(whilethread<block_amount){
      argumentosthread_compactacion* args = malloc(sizeof(argumentosthread));
      args->bolean=0;
      args->hecho=0;
      args->ruta = strdup(regruta[whilethread].line);
      args->key=key;
      args->cond = &cond;
      args->lock = lock;
      args->tabla=strdup(tabla);
      args->part=part;
      args->new_row=strdup(row_list[q].line);
      args->number_of_running_threads = number_of_threads;
      parametros[whilethread] = args;
      pthread_create(&buscadores[whilethread],NULL,buscador_compactacion,args);
      pthread_detach(buscadores[whilethread]);
      log_info(logger,"vuelta de armada de parametros");
      whilethread++;
    }
 
    log_info(logger,"antes del lock");
    pthread_mutex_lock(&lock);
    if(*number_of_threads!=0){
      log_info(logger,"espero condicion");
 
      pthread_cond_wait(&cond, &lock);
    }
    log_info(logger,"despues del lock");
 
    int whileparametro=0;
    int nada=0;
 
    while(whileparametro<block_amount){
 
      if(parametros[whileparametro]->hecho!=1){
        nada++;
      }
      whileparametro++;
    }
   
 
    if(nada==whileparametro){
      FILE* last=fopen(regruta[block_amount-1].line,"r+");
      fseek(last,0,SEEK_END);
      int size_file=ftell(last);
      int size_free=BLOCK_SIZE_DEFAULT-size_file;
      int size_row=strlen(row_list[q].line);
      if(size_row<size_free){
      log_info(logger,"despues del w");
        fputs(row_list[q].line,last);
        fclose(last);
        int tam_adjust=strlen(row_list[q].line);
        engine_adjust(tabla,part,tam_adjust);
      }
      else{
        fclose(last);
        new_block(row_list[q].line,tabla,part);
      }
    }
 
 
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);
    q++;
  }
  }
 
  return;
}
 
void* buscador_compactacion(void* args){
 
  argumentosthread_compactacion* parametros;
  parametros= (argumentosthread_compactacion*) args;
  FILE* bloque=NULL;
  log_info(logger,"comprobacion re parametros");
  log_info(logger,parametros->new_row);
  log_info(logger,parametros->ruta);
  int len_new_row=strlen(parametros->new_row);
  int length_row;
  void kill_thread(){
    pthread_mutex_lock(&parametros->lock);
    //*parametros->number_of_running_threads= *parametros->number_of_running_threads ;
    int amount = *parametros->number_of_running_threads;
    amount--;
    *parametros->number_of_running_threads= amount;
    if(amount==0) pthread_cond_broadcast(parametros->cond);
    pthread_mutex_unlock(&parametros->lock);
 
  }
 
  bloque=fopen(parametros->ruta,"r+");
 
  if(bloque==NULL){
    log_error(logger,"El sistema de bloques de archivos presenta una inconcistencia en el bloque:");
    log_error(logger,parametros->ruta);
    log_error(logger, "el archivo no existe.");
    kill_thread();
 
    return NULL;
  }
  fseek(bloque,0,SEEK_END);
  int block_size=ftell(bloque);
  rewind(bloque);
  int free_space=BLOCK_SIZE_DEFAULT-block_size;//max block;
  regg buffer[100];
  int l=0;
  parametros->retorno = strdup("");
  while(!feof(bloque)){
 
    buffer[l].line=malloc(100);
    buffer[l].line[0] = '\0';
    fgets(buffer[l].line,100,bloque);
    if(buffer[l].line[0] == '\0') break;
    parametros->row= strdup(buffer[l].line);
    buffer[l].dirty=0;
    if(parametros->key== get_row_key(buffer[l].line) ){
      log_info(logger,"se detecto key repetida");
      if(atoi(parametros->new_row)<atoi(buffer[l].line)){
      log_info(logger,"y con timestamp menor");
      length_row=strlen(buffer[l].line);
      parametros->bolean=1;
     
      if(len_new_row<=(free_space+length_row)){
      buffer[l].line=strdup(parametros->new_row);
      }
      else{
      buffer[l].dirty=1;
      }
      }
      else{
        parametros->hecho=1;
        fclose(bloque);
        kill_thread();
        return NULL;
      }
    }
    parametros->retorno = strdup("");
    l++;
    log_info(logger,"una vuela de lectura");
    }
    log_info(logger,"se termino de leer el bloque");
    rewind(bloque);
    fclose(bloque);
  if(parametros->bolean){
    bloque=fopen(parametros->ruta,"w");
    int contadorcito=0;
    int escrito=0;
    while(contadorcito<l){
    if(buffer[contadorcito].dirty!=1){
    log_info(logger,"se va a escribir:");
    log_info(logger,buffer[contadorcito].line);
    fputs(buffer[contadorcito].line,bloque);
    escrito++;
    }
    else{
    int lost=0 - strlen(buffer[contadorcito].line);
    engine_adjust(parametros->tabla,parametros->part,lost);
    }
    contadorcito++;
    }
    fflush(bloque);
    fclose(bloque);
    if(escrito==contadorcito){
    parametros->hecho=1;
    int lost2= len_new_row - length_row;
    engine_adjust(parametros->tabla,parametros->part,lost2);
    }
    kill_thread();
    pthread_cond_broadcast(parametros->cond);
   
    return NULL;
  }
 
  kill_thread();
  log_info(logger, "salida de la funcion");
  return NULL;
}
 
void* buscador(void* args){
  argumentosthread* parametros;
  parametros= (argumentosthread*) args;
  FILE* bloque=NULL;
 
  void kill_thread(){
   
    pthread_mutex_lock(&parametros->lock);
    int amount = *parametros->number_of_running_threads;
    amount--;
    *parametros->number_of_running_threads= amount;
    pthread_mutex_unlock(&parametros->lock);
    if(amount==0){
      if(parametros->cond == NULL) return;
      pthread_cond_broadcast(parametros->cond);
      pthread_mutex_destroy(&parametros->lock);
      pthread_cond_destroy(parametros->cond);
    }
  }
 
  bloque=fopen(parametros->ruta,"r+");
  if(bloque==NULL){
    log_error(logger,"El sistema de bloques de archivos presenta una inconcistencia en el bloque:");
    log_error(logger,parametros->ruta);
    log_error(logger, "el archivo no existe.");
    kill_thread();
    return NULL;
  }
 
  char *buffer = malloc(50);
  parametros->retorno = strdup("");
  while(!feof(bloque)){
    fgets(buffer,50,bloque);
    parametros->row= strdup(buffer);
 
    //devuelve key
 
    if(parametros->key==get_row_key(parametros->row)){
      obtengovalue(parametros->row,parametros->value);
      parametros->bolean=1;
      kill_thread();
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
 
 
int contar_rows(char* ruta){
  FILE* last_block=fopen(ruta,"r");
  int row_amount=0;
  char *trash = malloc(30);
  while(!feof(last_block)){
    fgets(trash,30,last_block);
    row_amount++;
  }
  fclose(last_block);
  return row_amount;
}
 
void adjust_size(char* size,int tam){
  char aux_size [10];
  int i =5;
  int p=0;
  log_info(logger,"antes de recorrer string");
  log_info(logger,size);
  while(size[i]!='\n' && size[i]!='\0'){
    aux_size[p]=size[i];
    i++;
    p++;
  }
  aux_size[p]='\0';
  char* check=string_itoa(i);
  log_info(logger,check);
  log_info(logger,aux_size);
  int tam2=atoi(aux_size);
  tam2= tam2+tam;
  char* aux=string_itoa(tam2);
  log_info(logger,aux);
  char* final="SIZE=";
  strcpy(size,final);
  strcat(size,aux);
  int last_tam=strlen(size);
  size[last_tam]='\n';
  size[last_tam+1]='\0';
  log_info(logger,final);
  return;
}
 
char* action_gossip(char* buffer){
  return strdup("");
}
 
void exec_err_abort(){};