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
<<<<<<< HEAD
//set up confg
=======
#include "../pharser.h"
#include "../actions.h"
#include "../console.h"

//punto de entrada para el programa y el kernel
t_log* logger;
int main(int argc, char const *argv[])
{
    
    //set up confg
>>>>>>> 230acc2e95961e2b222150dc747c1bb522e11d78
    t_config* config = config_create("config");
    char* LOGPATH = config_get_string_value(config, "LOG_PATH");
    //set up log
    pthread_t tid;
    logger = log_create(LOGPATH, "Filesystem", 1, LOG_LEVEL_INFO);

//punto de entrada para el programa y el kernel
int main(int argc, char const *argv[])
{    
    
    typedef struct{
  char consistency [2];
  int apartitions;
  int compactiont;
    }metadatareg;
    
    int PORT = config_get_int_value(config, "PORT");
   
   typedef struct{
   int keey
   char value [20]
   }readrow;
    


    //set up server
    server_info* serverInfo = malloc(sizeof(server_info));
    memset(serverInfo, 0, sizeof( server_info));    
    serverInfo->logger = logger;
    serverInfo->portNumber = PORT;

    pthread_create(&tid, NULL, create_server, (void*) serverInfo);

    //JOIN THREADS
    pthread_join(tid,NULL);
    
    //FREE MEMORY
    free(LOGPATH);
    free(logger);
    free(serverInfo);
    config_destroy(config);

      return 0;
}

//IMPLEMENTACION DE ACCIONES (Devolver error fuera del subconjunto)

char* obtener_key(char* nombreTabla, int key){
  //en esta funcion deberias bscar sobre tus archivos y mem table
  //y devolver ese resultado
  return string_new();
}

/* esta funcion es llamada automaticamnte por el servidor.
el return de la funcion es lo que devuelve el fs.
recibe por parametro un select info definido en server.h*/
char* action_select(package_select* select_info){
  log_info(logger, "Se recibio una accion select");
  char* auxrow;
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
return row;
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

void action_add(package_add* add_info){
  log_info(logger, "Se recibio una accion select");
}

char* action_run(package_run* run_info){
  log_info(logger, "Se recibio una accion run");
}

void action_metrics(package_metrics* metrics_info){
  log_info(logger, "Se recibio una accion metrics");
}


