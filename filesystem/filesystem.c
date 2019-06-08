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
  strcat(finalmetadata,"/metadata.txt");
  //imprimo la ruta por pantalla
  log_info(logger,finalmetadata);
  metadata=fopen(finalmetadata,"r+");
  //la ruta impresa es correcta pero no encuentra el archivo aunque exista
  if(metadata==NULL){
    log_info(logger,"no se encontro la metadata");
    return "metadata no encontrada";
  }
log_info(logger,"se encontro la metadata!");
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


  return "yes";

}
    /*char* auxrow;
    FILE* table=NULL;
    FILE* metadata=NULL;
    readrow rowreg;
    int finded=0;
    char* ruta=config_get_string_value(config,dir);
    int total= strlen(ruta)+strlen(select_info->table_name)+13;
    char* finalmetadata=maloc(total);
    strcat(finalmetadata,ruta);
    strcat(finalmetadata,select_info->table_name);
    strcat(finalmetadata,"metadata.txt");
    finalmetadata[total]='\0';
    metadata=fopen(finalmetadata,r+);
    metadatareg reading;
    while(!feof(metadata)){
    fread(reading,sizeof(metadatareg),1,metadata);
    }
    fflush(metadata);
    fclose(metadata);
    //aca obtendria la posicion del vector donde esta el registro deseado
    // con ese debo crear la ruta al .bin desado y sabria cual archivo abrir
    int part= select_info->key % reading.apartitions;
    char* ruta2= config_get_string_value(config,dir);
    int totalen= strlen(ruta2)+strlen(select_info->name_table)+1 +strlen((char* select_info->key)+5);
    char* final=malloc(totalen);
    strcat(final,ruta2);
    strcat(final,(char*)part);
    strcat(final,".bin");
    final[totalen]='\0';
    table=fopen(final,r+b);
  while(finded!=1 && !feof(table)){
  fgets(auxrow,100,table);
  if(atoi(auxrow)=select_info->key){
  finded=1;
    }
    }
  if(finded!=1){
    log_info(logger,"no se encontro la row solicitada");
    return "row no encontrada";
  }
  char* trash= get_string_from_buffer(auxrow,0);
  int index=strlen(trash);
  char* trash2= get_string_from_buffer(auxrow,index+1);
  int index2=strlen(trash2);
  char* row=get_string_from_buffer(auxrow,index+index2+1);
  }
  return row;*/


//TERMINADO
char* action_insert(package_insert* insert_info){

  //t_config* config = config_create("config"); 
  //consultar equipo si tengo que abrir config acá tambien.
  //el punto de montaje es estatico no hace falta
  //borre todos los mTN point salvo el primeroy lo hice global
  
  char* table_name = insert_info->table_name;
  //char* table_path = strdup("");  //lo convierto a puntero para no tener problema de tamaño
  char* table_path = malloc(sizeof(table_name)+sizeof(MNT_POINT)+sizeof("Tables/"));
  table_path[0] = '\0';
  
  strcat(table_path ,MNT_POINT);
  strcat(table_path ,"Tables/");
  strcat(table_path ,table_name);

  log_info(logger, table_path);

  log_debug(logger, table_path);//logea para saber que no la estas bardeando
  //verificar que la tabla exista en el fileSystem, en caso q no exista informar y continuar.
  //conviene tener todos los directorios abiertos para no bloquearse abriendo
  //nada
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

  //obtener la metadata asociada a dicha tabla (tabla1) ??Para que necesito la metadata???
  //si vos no sabes...
 
  insert_to_memtable(insert_info);

  //el parametro timestamp es opcional, en caso de que un request no lo provea, se usara el valor actual de epoch
  //EL TIMESTAMP ACA TIENE QUE ESTAR SI O SI!!!!!!
  
  //TODO insertar en la memoria temporal una nueva entrada que contanga los datos del request
  //esa es la memtable...
  //debug y
  log_debug(logger, "Se inserto el valor en la memtable");

  //char* response = "INSERT ok"; imaginate que cada vez que 
  //quieras abrir un programa te aparezca un cartel diciendo "se abrio correctamante"

  free(table_path);

  return "";
  
}

char* action_create(package_create* create_info){
  log_info(logger, "Se recibio una accion create");
  DIR* directori;
  FILE * metadata= NULL;
  char * nombre=create_info->table_name;
  char * ruta="MountTest/Tables/";
  directori= opendir(ruta);
  struct dirent * direntp;
  int iguales=1;
  log_info(logger,"directorio abierto");
  while((direntp=readdir(directori)) != NULL && iguales!=0){
    iguales=strcmp(direntp->d_name,create_info->table_name);
  }
  if(iguales==0){
    log_info(logger,"la tabla ya existe");
    return "la tabla ya existe";
  }
  char* guardado=malloc(50);
  log_info(logger,"malockeado");
  strcpy(guardado,"MountTest/Tables/");
  strcat(guardado,nombre);
  log_info(logger,guardado);
  mkdir(guardado,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  char* guardado2= strdup(guardado);
  strcat(guardado,"/metadata.txt");
  log_info(logger,"hasta aca");

metadata=fopen(guardado,"w+");
if(metadata!=NULL){
log_info(logger, "milagro");
}

char* constante=strdups(string_itoa(create_info->partition_number));
int util=strlen(constante)+strlen("particiones");
char* partitions=malloc(util +3);
strcpy(partitions,"\n");
strcpy(partitions,constante);
strcat(partitions,"=particiones\n0");
log_info(logger,partitions);
char* constante2= strdups(string_itoa(create_info->compactation_time));
int util2=strlen(constante2)+strlen("tiempo de compactacion");
char* compactation= malloc(util2+3);
strcpy(compactation,"\n");
strcpy(compactation,constante2);
strcat(compactation,"=tiempo de compactacion");
log_info(logger,compactation);
char* constante3= strdup(string_itoa(create_info->consistency));
char* consistencia;
int larg0=strlen("sc=consistency");
consistencia= malloc(larg0+5);
switch(atoi(constante3)){
case 0:
strcpy(consistencia,"sc=consistency\n0");
break;
case 1:
strcpy(consistencia,"sh=consistency");
break;
case 2:
strcpy(consistencia,"ev=consistency");
break;
}
log_info(logger,consistencia);
fputs(consistencia,metadata);
fputs(partitions,metadata);
fputs(compactation,metadata);
int c= create_info->partition_number;
c--;

char* resp=malloc(100);
while(c>=0){
char* auxx=NULL;
auxx=strdup(string_itoa(c));
strcpy(resp,guardado2);
strcat(resp,"/");
strcat(resp,auxx);
strcat(resp,".bin");
log_info(logger,resp);
 fopen(resp,"w+");
  c--;
}
 fclose (metadata); 
 closedir(directori);
 free(guardado);
 free(partitions);
 free(consistencia);
 free(resp);
return "si";
}

char* action_describe(package_describe* describe_info){
  log_info(logger, "Se recibio una accion describe");

  /*
  //defino el punto de partida para recorrer las tablas
  char path_table[300];
  path_table[0] = '\0';
  strcat(path_table,MNT_POINT);
  strcat(path_table,"Tables/");

  char** t_names;
  int t_count;
  int flag = 0; //indica si hubo una tabla a describir

  //distingo si cargaron o no una tabla a describir
  if (describe_info->table_name != NULL) {
    
    //TODO si la tabla no existe
    log_error(logger,"tomo este caminoo");
    
    t_names = malloc(sizeof(char *));
    t_names[0] = (char *) malloc(256);
    t_names[0] = describe_info->table_name;
    t_count = 1;
    flag = 1;

  } else {

    log_error(logger,"tomo este otro");

    t_names = tables_names(); //obtengo los nombres de todas las tablas
    t_count = tables_count(); //calculo cuantas tablas hay
  }

  char buff[3000];
  buff[0] = '\0';
  
  int i;
  for(i=0; i<t_count; i++){ //recorro las tablas y extraigo la metadata
    char* n = t_names[i];

    strcat(buff,"DESCRIBE");
    strcat(buff,"\n");
    strcat(buff,n);
    strcat(buff,"\n");

    char path_metadata[300];
    snprintf(path_metadata, 300, "%s", path_table);

    //aca leo la metadata de cada tabla que tengo
    strcat(path_metadata,n);
    strcat(path_metadata,"/Metadata.bin"); //path de metadata de la tabla

    //leo y cargo la metadata en buffer
    FILE* fp_m = fopen(path_metadata,"r");

    char consistency[300];
    char partitions[300];
    char compaction_time[300];

    fgets(consistency, 300, fp_m);
    fgets(partitions, 300, fp_m);
    fgets(compaction_time, 300, fp_m);
    
    strcat(buff,consistency);
    strcat(buff,partitions);
    strcat(buff,compaction_time);
    strcat(buff,"\n");

    fclose(fp_m);

    path_metadata[0] = '\0';

  } 

  log_info(logger,buff);

  if (flag == 1) {
    free(t_names[0]);
    free(t_names);
  }

  //retornar el contenido de dichos archivos de metadata
  return "buff";//no podes devolver algo que no sea puntero
  */
return "";
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