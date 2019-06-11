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

//punto de entrada para el programa y el kernel
t_log* logger;

//crear memtable
char* MNT_POINT;
void buscador(char* ruta,int key, char* retorno, char* row);
char *strdups(const char *src);
int contarbloques(char*);
void vaciarvector(char* puntero);
void leerarchivo(FILE* metadata, regg* regmetadata);
void obtenerbloques(char* pointer1, int* pointer2);
void vaciadobuffer(char* buffer);
void cortador(char* cortado, char* auxkey);

int main(int argc, char const *argv[])
{
  
    //las estructuras se van al .h para que quede mas limpio
    //set up confg
    t_config* config = config_create("config");
    char* LOGPATH = config_get_string_value(config, "LOG_PATH");
    MNT_POINT = config_get_string_value(config, "PUNTO_MONTAJE");
    int PORT = config_get_int_value(config, "PORT");

    //set up log
    logger = log_create(LOGPATH, "Filesystem", 1, LOG_LEVEL_INFO);

    engine_start(logger);
    enginet_create_table("tabla sc con nombre largo", S_CONSISTENCY, 7, 25555555);
    enginet_create_table("a145", H_CONSISTENCY, 4, 25555555);
    enginet_create_table("tabla4", ANY_CONSISTENCY, 26, 8000000);

    
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

int tables_count() {
  
  //defino el punto de partida para recorrer las tablas
  char *path_table = strdup("");
  strcat(path_table,MNT_POINT);
  strcat(path_table,"Tables/");

  int dir_count = 0;
  struct dirent* dent;
  DIR* srcdir = opendir(path_table); //abro el directorio /Tables

  if(srcdir == NULL) {
    log_error(logger, "No se pudo abrir el directorio /Tables");
  }
  while((dent = readdir(srcdir)) != NULL) { //mientras el directorio no este vacio
    struct stat st;
    if(strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0) {
      continue;
    }
    if(fstatat(dirfd(srcdir), dent->d_name, &st, 0) < 0) {
      log_error(logger, dent->d_name);
      continue;
    }
    if(S_ISDIR(st.st_mode)) {
      dir_count++;
      //log_info(logger,dent->d_name);
    }
  }
  closedir(srcdir);

  return dir_count;
}

char** tables_names(){
  int tables_cant = tables_count();
  char** names = malloc(tables_cant * sizeof(char *));

  //aloco memoria para cada nombre de tabla
  int i;
  for (i=0; i<tables_cant; i++) {
    names[i] = (char *) malloc(256);
  }
  
  //cargo todos los nombres
  
  //defino el punto de partida para recorrer las tablas
  char path_table[300];
  path_table[0] = '\0';
  strcat(path_table,MNT_POINT);
  strcat(path_table,"Tables/");

  int j = 0;
  struct dirent* dent;
  DIR* srcdir = opendir(path_table); //abro el directorio /Tables

  if(srcdir == NULL) {
    log_error(logger, "No se pudo abrir el directorio /Tables");
  }
  while((dent = readdir(srcdir)) != NULL) { //mientras el directorio no este vacio
    struct stat st;
    if(strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0) {
      continue;
    }
    if(fstatat(dirfd(srcdir), dent->d_name, &st, 0) < 0) {
      log_error(logger, dent->d_name);
      continue;
    }
    if(S_ISDIR(st.st_mode)) {
      names[j] = dent->d_name;
      j++;
      //log_info(logger,dent->d_name);
    }
  }
  closedir(srcdir);

  //retorno el resultado
  return names;

  //libero todos los punteros
  for (i=0; i<tables_cant; i++) {
    free(names[i]);
  }
  free(names);
}

//ruta correcta pero no abre la metadata
char* action_select(package_select* select_info){
    log_info(logger, "Se recibio una accion select");
    //te juro que lo intente pero no se que haces aca
    //la idea del engine es no tener que levantar el punto de montaje por
    //cada select. al ser de alta disponibilidad no tendria que detenerse nunca a mirar
    //en que bloque esta

    if(!does_table_exist(select_info->table_name)){
      return "La tabla solicitada no existe.\n";
    }


    //defino variables a usar
    char* auxpart;
    char* auxkey;
    char* row;
    FILE* table=NULL;
    FILE* metadata=NULL;
    regg regmetadata[2];
    regg regpart[3];
    int numerobloques;
    int contador= 0;
    int contador2=0;
    int cont=0;
    int finded=0;
    //construyo la ruta para abrir el archivo metadata
    char* ruta = "MountTest/";
    char* rutaa= "MountTest/Tables/";//punto de montaje dudoso, arreglar
    int total= strlen(rutaa)+strlen(select_info->table_name)+13;
    char* finalmetadata=malloc(100);
    strcpy(finalmetadata,rutaa);
    char* nombre=strdup(select_info->table_name);
    strcat(finalmetadata,nombre);
    strcat(finalmetadata,"/metadata");
    //imprimo la ruta por pantalla
    log_info(logger,finalmetadata);
    metadata=fopen(finalmetadata,"r+");
    //la ruta impresa es correcta pero no encuentra el archivo aunque exista
    //ahora lo encuentra. No se toma el hecho de que no lo encuentre
    //xq no puede pasar
    /*if(metadata==NULL){
      log_info(logger,"no se encontro la metadata");
      return "metadata no encontrada";
    }*/
  log_info(logger,"se encontro la metadata!");
  //SUPONE QUE LA METADATA YA LA TENES

  char charcito[100];
  while(!feof(metadata)){
  fgets(charcito,100,metadata);
  regmetadata[contador].line=strdups(charcito);
  contador++;
  }
  fclose(metadata);
  int partition=atoi(regmetadata[1].line);
  char* check1=strdup(string_itoa(partition));
  char* check2=strdup(string_itoa(select_info->key));
  log_info(logger,check1);
  log_info(logger,check2);
  int part= select_info->key % partition;
  auxkey=strdups(string_itoa(part));
  log_info(logger,auxkey);
  int totalen= strlen(rutaa)+strlen(nombre)+1 +strlen(auxkey)+5;
  char* final=malloc(100);
    strcpy(final,rutaa);
    strcat(final,nombre);
    strcat(final,"/");
    log_info(logger,"memoria malockeada");
    auxpart=strdups(string_itoa(part));
    strcat(final,auxpart);
    strcat(final,".txt");
    table=fopen(final,"r+");
    if(table==NULL){
      log_info(logger,final);
      return "tabla no encontrada";
    }
    
    log_info(logger,"particion encontrada");
  char* charcito2= malloc(100);
  while(!feof(table)){
    fgets(charcito2,100,table);
    regpart[contador2].line=strdups(charcito2);
    contador2++;
  }
  log_info(logger, "hasta aca!");
  numerobloques=contarbloques(regpart[1].line);
  char* precaucion;
  precaucion= strdup(string_itoa(numerobloques));
  log_info(logger,precaucion);
  int intarray[numerobloques];
  obtenerbloques(regpart[1].line,intarray);
  //vos estas mal de la cabeza jajajaj
  pthread_t buscadores[numerobloques];
  regg regruta[numerobloques];
  char* test1=strdup(string_itoa(intarray[0]));
  char* test2=strdup(string_itoa(intarray[1]));
  char* test3=strdup(string_itoa(intarray[2]));
  log_info(logger,test1);
  log_info(logger,test2);
  log_info(logger,test3);
  log_info(logger,"hasta aca!");
  log_info(logger,ruta);
  int sus=0;
  while(sus<numerobloques){
  regruta[sus].line=malloc(100);
  strcpy(regruta[sus].line,ruta);
  strcat(regruta[sus].line,"Bloques/");
  char* auxb=strdup(string_itoa(intarray[sus]));
  strcat(regruta[sus].line,auxb);
  strcat(regruta[sus].line,".bin");
  log_info(logger,regruta[sus].line);
  sus++;
  }


  return "no?";

}


char* action_insert(package_insert* insert_info){

  char* table_name = insert_info->table_name;
  char* table_path = malloc(sizeof(table_name)+sizeof(MNT_POINT)+sizeof("Tables/"));
  table_path[0] = '\0';
  
  strcat(table_path ,MNT_POINT);
  strcat(table_path ,"Tables/");
  strcat(table_path ,table_name);

  log_info(logger, table_path);

  //ya tengamos las tablas guardadas para mayor eficiencia
  //(preguntar al engine y memtable)(por separado?)

  DIR* dir = opendir(table_path);
  if (dir) {
    closedir(dir);
  } else if (ENOENT == errno) {
    //directorio no existe
    log_error(logger, "La tabla indicada no existe");
    return "";
  } else {
    //opendir() fallo por otro motivo
    log_error(logger, "Error al verificar la existencia de la tabla, fallo opendir()");
    return "";
  }
 
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

  char* result = get_all_tables_metadata();
  
  return result;
}

char* action_drop(package_drop* drop_info){
  log_info(logger, "Se recibio una accion drop");
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
int contarbloques(char* pointer1){
int pos=8;
int cantidadbloques=0;
while(pointer1[pos]!=']'){
if(pointer1[pos]==','){
cantidadbloques++;
}
pos++;
}
cantidadbloques++;
return cantidadbloques;
}

void obtenerbloques(char* pointer1, int* pointer2){
int pos=8;
int i=0;
char buffer[5];
int vec= 0;
while(pointer1[pos]!=']'){
if(pointer1[pos]==','){
pos++;
}
while(pointer1[pos]!=',' && pointer1[pos]!=']') {
buffer[i]= pointer1[pos];
i++;
pos++;
}
pointer2[vec]=atoi(buffer);
vec++;
i=0;
vaciadobuffer(buffer);
}
return;
}

void vaciadobuffer(char* buffer){
for(int i=0;i<5;i++){
buffer[i]='\0';
}
return;
}


void vaciarvector(char* puntero){
  for(int i=0;i<100;i++){
  puntero[i]='\0';
}
return;
}

void buscador(char* ruta,int key, char* retorno, char* row){
FILE* bloque=NULL;
bloque=fopen(ruta,"r+");
if(bloque==NULL){
  log_info(logger,"no se encontro el bloque en");
  log_info(logger,ruta);
  return;
}
char buffer[100];
vaciadobuffer(retorno);
while(!feof(bloque)){
   fgets(buffer,100,bloque);
    char* row= strdup(buffer);
cortador(buffer,retorno);
if(key==atoi(retorno)){
    log_info(logger,"encontrada");
    return;
}
vaciadobuffer(retorno);
}
log_info(logger, "no es este bloque");
return;
}

void cortador(char* cortado, char* auxkey){
int i=0;
int j=0;
while(cortado[i]!=' ' &&cortado[i]!='\n'){
      i++;
      }
i++;

while(cortado[i]!=' ' && cortado[i]!='&'){
     auxkey[j]=cortado[i];
     i++;
     j++;
      }
return;
}