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
#include "segments.h"

//logger global para que lo accedan los threads
t_log* logger;
int fs_socket;
int main_memory_size;


//punto de entrada para el programa y el kernel
int main(int argc, char const *argv[])
{
  //set up config  
  t_config* config = config_create("config");
  char* LOGPATH = config_get_string_value(config, "LOG_PATH");
  int PORT = config_get_int_value(config, "PORT");

  //set up log
  logger = log_create(LOGPATH, "Memory", 1, LOG_LEVEL_INFO);
  log_info(logger, "El log fue creado con exito\n");

  //set up server
  pthread_t tid;
  server_info* serverInfo = malloc(sizeof(server_info));
  memset(serverInfo, 0, sizeof(server_info));    
  serverInfo->logger = logger;
  serverInfo->portNumber = PORT; 
  int reslt = pthread_create(&tid, NULL, create_server, (void*) serverInfo);

  //set up client 
  fs_socket = socket(AF_INET, SOCK_STREAM, 0);
  char* FS_IP = config_get_string_value(config, "FS_IP");
  int FS_PORT = config_get_int_value(config, "FS_PORT");
  printf("%s %d\n", FS_IP, FS_PORT);
  struct sockaddr_in sock_client;
  
  sock_client.sin_family = AF_INET; 
  sock_client.sin_addr.s_addr = inet_addr(FS_IP); 
  sock_client.sin_port = htons(FS_PORT);

  int connection_result =  connect(fs_socket, (struct sockaddr*)&sock_client, sizeof(sock_client));
  
  if(connection_result < 0){
    log_error(logger, "No se logro establecer la conexion con el File System");   
  }
  else{
  }

  main_memory_size = config_get_int_value(config, "TAM_MEM");
  MAIN_MEMORY = malloc(main_memory_size);
  memset(MAIN_MEMORY, 0, main_memory_size);

  SEGMENT_TABLE = NULL;
  NUMBER_OF_PAGES = main_memory_size / sizeof(page_t); // la cantidad de paginas que voy a tener con el tamano actual de cada pagina
  LRU_TABLE = create_LRU_TABLE();

  printf("\n---- Memory info ----\n");
  printf("Main memory size: %d\n", main_memory_size);
  printf("Number of pages: %d\n", NUMBER_OF_PAGES);
  printf("---------------------\n\n");
  

  // -------------- PRUEBAS --------------

  // segment_t* segment1 = create_segment("tabla1");
  // segment_t* segment2 = create_segment("tabla2");
  // page_t* new_page1 = create_page(123, 42, "holis");
  // page_t* new_page2 = create_page(321, 69, "chau");

  // segment_t* s1 = find_or_create_segment("tabla1");
  // page_info_t* page_info1 = save_page(s1, new_page1);
  // page_info_t* page_info2 = save_page(s1, new_page2);

  // print_segment_pages(s1);

  // lru_page_t* lru_page_info1 = create_lru_page(s1,page_info1); 
  // lru_page_t* lru_page_info2 = create_lru_page(s1,page_info2); 


  // -------------- FIN PRUEBAS --------------


  //inicio lectura por consol
  pthread_t tid_console;
  pthread_create(&tid_console, NULL, console_input, "Memory");

  //Espera a que terminen las threads antes de seguir
  pthread_join(tid,NULL);
  
  //FREE MEMORY
  free(LOGPATH);
  free(logger);
  free(serverInfo);
  config_destroy(config);

  return 0;
}


//IMPLEMENTACION DE FUNCIONES (Devolver errror fuera del subconjunto)

char* action_select(package_select* select_info){
  log_info(logger, "Se recibio una accion select");

  //BUSCO O CREO EL SEGMENTO
  // tengo 
  segment_t* segment = find_or_create_segment(select_info->table_name); // si no existe el segmento lo creo.
  //SI EXISTE LA PAGINA:
  page_info_t* page_info = find_page_info(segment, select_info->key); // cuando creo paginas en el main y las busco con la misma key, no me las reconoce por alguna razon
  if(page_info != NULL){
    printf("Page found in memory -> Key: %d, Value: %s\n", select_info->key, page_info->page_ptr->value);
    return page_info->page_ptr->value;
  }
  //SI NO EXISTE LA PAGINA:
  // ENVIO AL FILESYSTEM
  char* packageTemp = parse_package_insert(select_info);
  char* responce = exec_in_fs(fs_socket, packageTemp); 
  if(responce =! "Key invalida."){
    page_t* page = create_page(007, select_info->key, responce); //CUIDADO TIMESTAMP
    save_page(segment, page);
    printf("Page found in file system -> Key: %d, Value: %s\n", page->key, page->value);
    return page->value;
  }
}
//Necesito saber si es Timestamp lo genera memoria o el Kernel antes de enviarlo.
// De no haberse ya generado el TS reemplazo <insert_info->timestamp> por <(unsigned)time(NULL)>... CREO.

char* action_insert(package_insert* insert_info){
  log_info(logger, "Se recibio una accion insert");

 //BUSCO O CREO EL SEGMENTO
   segment_t*  segment = find_or_create_segment(insert_info->table_name); // si no existe el segmento lo creo.
   printf("Table name: %s\n", segment->name);
 //SI EXISTE LA PAGINA:
   page_t* page = find_page(segment, insert_info->key);
   if(page != NULL){ //Faltaria && <(insert_info->timestamp > page->timestamp)>??
     page->timestamp = insert_info->timestamp;
     page->value = insert_info->value; //TODO: ARREGLAR ESTE PROBLEMA
     printf("Page edited> Key: %d, Value: %s\n", insert_info->key, page->value);
     return page; // TODO: Retornar algo correcto
   }
 //SI NO EXISTE LA PAGINA:
   page_t* page = create_page(insert_info->timestamp, insert_info->key, insert_info->value); 
   save_page(segment, page);
   printf("Page created-> Key: %d, Value: %s\n", page->key, page->value);
   return page; // TODO: Retornar algo correcto
}

//en esta funcion se devuelve lo 
//proveniente del gossiping
//devuelve solo las seeds
//con esta forma: RETARDO_GOSSIPING_DE_ESTA_MEMORIA|id,ip,port|id,ip,port|id,ip,port
//                                                    seed        seed      seed
char* action_intern__status(){
  char* res = strdup("300000000|"); //ya que se puede modificar en tiempo real y yo necesito saber cada cuanto ir a buscar una memoira se le añade como primer elemento el retargo gossiping de la memoria principal.
  char sep[2] = { ',', '\0' };
  char div[2] = { '|', '\0' };
  t_config* config = config_create("config");
  char** ips = config_get_array_value(config, "IP_SEEDS");
  char** ports = config_get_array_value(config, "PUERTO_SEEDS");

  int i = 2; //ESto reprecenta el id de las memorias, se consigue como parte del proceso de gossiping.
  while(*ips != NULL){
    strcat(res, string_itoa(i));
    strcat(res,sep);    
    strcat(res, *ips);
    strcat(res,sep);
    strcat(res, *ports);
    strcat(res,div);
    ips++;
    ports++;
    i++;
  }
  return res;
}

char* action_create(package_create* create_info){
  log_info(logger, "Se recibio una accion create");
  char* responce = exec_in_memory(fs_socket, create_info); 
  return responce;
}

char* action_describe(package_describe* describe_info){
  log_info(logger, "Se recibio una accion describe");
  char* responce = exec_in_memory(fs_socket, describe_info); 
  return responce;
}

char* action_drop(package_drop* drop_info){
  log_info(logger, "Se recibio una accion drop");
  void remove_segment(char* table_name);//ELIMINA LA TABLA
  char* responce = exec_in_memory(fs_socket, drop_info); 
  return responce; 
}


char* action_journal(package_journal* journal_info){
  log_info(logger, "Se recibio una accion select");

//VOY AL ULTIMO SEGMENTO
  segment_t* segmentTemp = get_last_segment();
  int contador = 0;
  while(segmentTemp != NULL){
    //VOY A LA ULTIMA PAGINA
    page_info_t* pageTemp =  get_last_page(segmentTemp -> pages);
    
    while(segmentTemp != NULL){
      //CHEQUEO DIRTY BIT EN 1
      if(pageTemp -> dirty_bit = 1){
        //ENVIO AL FILESYSTEM
        package_insert* insertTemp;
        insertTemp->table_name = segmentTemp->name;
        insertTemp->key = pageTemp->page_ptr->key;
        insertTemp->value = pageTemp->page_ptr->value;
        insertTemp->timestamp = pageTemp->page_ptr->timestamp;
       
        //FS_SOCKET es global e unica?

        char* packageTemp = parse_package_insert(insertTemp);
        char* responce = exec_in_memory(fs_socket, packageTemp); 
 
      //  unlock_memory(fs_socket);
         return responce;
       
        contador++;
      }
      //ELIMINO PAGINA Y REDIRECCIONO A LA PREVIA
      page_info_t* pageTemp2 = pageTemp;
      remove_from_segment(segmentTemp, pageTemp);
      pageTemp = pageTemp2 -> prev;
    }
    //REDIRECCIONO A SEGMENTO PREVIO
    segment_t * segmentTemp2;
    segmentTemp2 = segmentTemp;
    segmentTemp = segmentTemp -> prev;
    //ELIMINO EL SEGMENTO ANTERIOR
    remove_segment(segmentTemp2);
  }
  printf("Journal finalizado, Paginas enviadas a FS : %d \n", contador);
}


char* action_add(package_add* add_info){
  log_info(logger, "Se recibio una accion select");
}

char* action_run(package_run* run_info){
  log_info(logger, "Se recibio una accion run");
}

char* action_metrics(package_metrics* metrics_info){
  log_info(logger, "Se recibio una accion metrics");
}

char* parse_input(char* input){
  return exec_instr(input);
}

/*

--FEO--

char* action_journal(package_journal* journal_info){
  log_info(logger, "Se recibio una accion select");

//LEO TODOS LOS SEGMENTOS
	segment_t* tempSegment = SEGMENT_TABLE;
	while(tempSegment != NULL){
	//LEO TODAS LAS PAGINAS	
    page_info_t* tempPage = tempSegment->pages
	  while(tempPage != NULL){
      //FILTRO LAS PAGINAS CON BIT MODIFICADO
		  if(tempPage->dirty_bit = 1){
        //FUNCION. send(tempPage->page_ptr, FS) ?? Como envio toda la estructura ((page_t* page_ptr)) ??
        }
		  }
		  tempPage = tempPage->next;
		}
		tempSegment = tempSegment->next;
	}

}
*/