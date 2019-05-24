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

//logger global para que lo accedan los threads
t_log* logger;
int fs_socket;

//declaro memoria principal
char* main_memory;
int main_memory_size;

int SEGMENT_SIZE = 1024; //TODO obtener el numero posta del handshake con fs
int PAGE_SIZE = 512;

typedef struct{
  char* name;
  char* base;
  int limit;
  int pages[1];
}segment_info;

typedef struct segment{
  segment_info data;
  struct segment *next;
}segment;

segment* create_segment(){
  segment* temp;
  temp = (segment*)malloc(sizeof(segment));
  temp->next = NULL;
  return temp;
}

segment* SEGMENT_TABLE;

//retorna el index del segmento que tiene suficiente memoria entre el mismo y el proximo segmento. -1 si no encuentra nada. Si tira -1 hay que hacer lugar.
int find_memory_space(int memory_needed){
  char* base_memory = &main_memory[0];  
  
  if(memory_needed > main_memory_size){
    return -1; //TODO hacer que tire error en vez de devolver -1 porque no hay manera de encontrar espacio.
  }

  //si SEGMENT_TABLE es null, entonces no hay segmentos por lo que memoria esta vacia. Asigno en el primer byte de memoria
  if(SEGMENT_TABLE == NULL){
    return 0;
  }

  segment* temp = SEGMENT_TABLE;
  int i = 1;
  while(temp != NULL){
    if(SEGMENT_TABLE->data.base - base_memory >= memory_needed){
      return i - 1;
    }
    base_memory = &temp->data.base[temp->data.limit]; //asigno la base de memoria como la base del segmento mas el desplazamiento para comparar el proximo segmento con esta base
    temp = temp->next;
    i++;
  }

  //estoy en el ultimo segmento, la diferencia entre el utlimo byte de memoria y la base_memory es el espacio de memoria que queda.
  return &main_memory[main_memory_size] - base_memory >= memory_needed ? i : -1;
}

void add_segment_to_table(int index, segment* new_segment){
  if(index == 0){
    segment* temp = SEGMENT_TABLE;
    SEGMENT_TABLE = new_segment;
    new_segment->next = temp;
  }
  else{
    segment* temp = SEGMENT_TABLE;
    int i = 1;
    while(i < index){ //itero sobre los segments hasta llegar al index
      temp = temp->next;
      i++;
    }
    new_segment->next = temp->next;
    temp->next = new_segment;
  }
}

void save_segment_to_memory(segment* new_segment){
  find_memory_space()
}


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

  //reservo memoria contigua para la memoria principal
  main_memory_size = config_get_int_value(config, "TAM_MEM");
  main_memory = malloc(main_memory_size);
  memset(main_memory, 0, main_memory_size);
  printf("%s: %p\n", "Puntero a memoria[0]", &main_memory[0]);
  printf("%s: %p\n", "Puntero a memoria[4096]", &main_memory[main_memory_size]);
  printf("%s: %d\n", "Tamanio memoria", &main_memory[main_memory_size] - &main_memory[0]);

  SEGMENT_TABLE = NULL;

  int asd = find_memory_space(400);
  //inicio lectura por consola
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
  log_info(logger, "Memory: Se recibio una accion select");
  int asd = find_memory_space(400);
  // printf("Segment index: %d\n", asd);
  char* response = parse_package_select(select_info);
  send(fs_socket, response, strlen(response)+1, 0);
  segment_info seg_info;
  seg_info.name = select_info.table_name;
  save_segment_to_memory(select_info.ta)
  segment* new_segment = create_segment();
  new_segment->data.

  return string_new("holis"); //tienen que devolver algo si no se rompe
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

char* parse_input(char* input){
  return exec_instr(input);
}
