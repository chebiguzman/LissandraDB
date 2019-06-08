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
#include "../memory_functions.h"

#define VALUE_SIZE 128

//logger global para que lo accedan los threads
t_log* logger;
int fs_socket;

//declaro memoria principal
char* main_memory;
int main_memory_size;

int PAGE_SIZE = 128;
int SEGMENT_SIZE = 1024; //TODO obtener el numero posta del handshake con fs

segment* SEGMENT_TABLE;

segment* create_segment(){
  segment* temp;
  temp = (segment*)malloc(sizeof(segment));
  temp->next = NULL;
  temp->data.pages[0] = -1;
  temp->data.pages[1] = -1;
  temp->data.pages[2] = -1;
  return temp;
}

segment* get_segment(int index){
    segment* temp = SEGMENT_TABLE;
    int i = 1;
    while(i < index){ //itero sobre los segments hasta llegar al index
      temp = temp->next;
      i++;
    }
    return temp;
}

void add_segment_to_table(int index, segment* new_segment){ //TODO guardar el value en memoria principal, hasta ahora solo guardo lo demas en la table
  if(index == 0){
    segment* temp = SEGMENT_TABLE;
    SEGMENT_TABLE = new_segment;
    new_segment->next = temp;
  } 
  else{
    segment* temp = get_segment(index);
    new_segment->next = temp->next;
    temp->next = new_segment;
  }
}

//devuelve la ultima direccion de memoria del segment, habria que agregarle -1 
char* get_end_memory_address(int index){
  if(index == 0){
    return &main_memory[0];
  }
  segment* temp = get_segment(index);
  return &temp->data.base[temp->data.limit - 1]; //base de memoria del segment con desplazamiento limit
}

// devuelve la direccion que el nuevo segmento deberia usar de base
char* get_first_memory_address_after(int index){
  if(index == 0){
    return &main_memory[0];
  }
  return get_end_memory_address(index) + 0x1;
}

//retorna el index del segmento que tiene suficiente despues del mismo y el proximo segmento. -1 si no encuentra nada. Si tira -1 hay que hacer lugar.
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
    if(temp->data.base - base_memory >= memory_needed){
      return i - 1;
    }
    base_memory = get_first_memory_address_after(i); //asigno la base de memoria como la base del segmento mas el desplazamiento para comparar el proximo segmento con esta base
    temp = temp->next;
    i++;
  }

  //estoy en el ultimo segmento, la diferencia entre el utlimo byte de memoria y la base_memory es el espacio de memoria que queda.
  //a[10] = [0..9]
  //a[size(a)-1] = 9
  //a[0] = 0
  //si se restan, dan 9, cuando el size es 10, hay que sumar 1
  return &main_memory[main_memory_size - 1] - base_memory + 1 >= memory_needed ? i - 1 : -1;
}
// TODO: corroborar que el [temp->data.limit - 1] y el get_end_memory_address(index) + 1 este ok

void save_segment_to_memory(segment_info segment_info){
  int index = find_memory_space(segment_info.limit);
  printf("Index to save segment info: %d\n", index);
  // busco espacio en memoria y me devuelve un index
  
  char* base_memory = get_first_memory_address_after(index);
  printf("Base del nuevo segmento %p\n", base_memory);
  
  // traduzco el index al address de memoria

  segment_info.base = base_memory;
  segment* new_segment = create_segment();
  new_segment->data = segment_info;
  add_segment_to_table(index, new_segment);
}

void print_segment_info(segment* temp){
  printf("Nombre de tabla: %s\n", temp->data.name);
  printf("Base de memoria: %p\n", temp->data.base);
  printf("Tamanio segmento: %d\n", &temp->data.base[temp->data.limit - 1] - temp->data.base + 1);
  printf("Ultima posicion de memoria: %p\n\n", &temp->data.base[temp->data.limit - 1]);
}

// devuelve el indice del segmento que contiene el nombre de tabla o -1 si no encuentra
int find_table(char* table_name){
  segment* temp = SEGMENT_TABLE;
  int index = 1;
  while(temp != NULL){
    if(strcmp(temp->data.name, table_name) == 0){
      return index;
    }
    index++;
    temp = temp->next;
  }
  return -1;
}

int find_page(segment* segment, int size, int key){
  for (int i = 0; i < size; i++){
    if(segment->data.pages[i] == key){
      return i;
    }
  }
  return -1;
}

int get_free_page(int pages[], int number_of_pages){
  for(int i = 0; i < number_of_pages; i++){
    if(pages[i] == -1){
      return i;
    }
  }
  return 0; //TODO: implementar algoritmo LRU, necesito timestamps?
}

void add_key_to_table(segment* segment, int index, int key){
  segment->data.pages[index] = key;
}

int get_memory_offset(char* base){
  return base - main_memory;
}

void free_memory_space(char* address, int size){
  memset(address, 0, size);
}

char* get_page_address(segment* segment, int page_index){
  int segment_offset = get_memory_offset(segment->data.base);
  int page_offset = page_index * PAGE_SIZE;
  int page_address = segment_offset + page_offset; // devuelve la ubicacion de la pagina (segmento + page_size)
  return main_memory+page_address;
}

// TODO: find page_index dinamically en vez de pasarla por parametro
// Busca la direccion de la pagina dentro del segmento y le copia el valor, devuelve esa direccion
char* save_value_to_memory(segment* segment, int page_index, char* value){
  char* page_address = get_page_address(segment, page_index);
  free_memory_space(page_address, PAGE_SIZE);
  memcpy(page_address, value, VALUE_SIZE); 
  return page_address;
}

void save_registry(segment* segment, int key, char* value){
  // int size = sizeof(segment->data.pages) / PAGE_SIZE;
  int index = get_free_page(segment->data.pages, 2); //TODO: cambiar el 2 por el size!
    if(index == -1){
      // JOURNALIAR ATR
    }
  add_key_to_table(segment, index, key);
  save_value_to_memory(segment, index, value);
}

char* get_value(segment* segment, int page_index){
  int page_offset =  page_index * PAGE_SIZE;
  int segment_base = get_memory_offset(segment->data.base);
  return main_memory+segment_base+page_offset;
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
  printf("%s: %p\n", "Puntero a memoria[4096]", &main_memory[main_memory_size -1]);
  printf("%s: %d\n\n", "Tamanio memoria", &main_memory[main_memory_size -1] - &main_memory[0] + 1);

  SEGMENT_TABLE = NULL;

  segment* segment1 = create_segment();
  segment_info seg_info1;
  seg_info1.limit = SEGMENT_SIZE;
  seg_info1.name = "tabla1";
  seg_info1.pages[0] = -1;
  seg_info1.pages[1] = -1;
  segment1->data = seg_info1;

  segment* segment2 = create_segment();
  segment_info seg_info2;
  seg_info2.limit = SEGMENT_SIZE;
  seg_info2.name = "tabla2";
  segment2->data = seg_info2;

  segment* segment3 = create_segment();
  segment_info seg_info3;
  seg_info3.limit = SEGMENT_SIZE;
  seg_info3.name = "tabla3";
  segment3->data = seg_info3;

  save_segment_to_memory(seg_info1);
  save_segment_to_memory(seg_info2);
  save_segment_to_memory(seg_info3);

  print_segment_info(get_segment(1));
  print_segment_info(get_segment(2));
  print_segment_info(get_segment(3));

  printf("Pages[0]: %d\n", get_segment(1)->data.pages[0]);
  printf("Pages[1]: %d\n", get_segment(1)->data.pages[1]);

  save_registry(get_segment(1), 3, "hola");
  save_registry(get_segment(1), 5, "chau");
  printf("Pages[0]: %d\n", get_segment(1)->data.pages[0]);
  printf("Pages[1]: %d\n", get_segment(1)->data.pages[1]);
  
  printf("Value: %s\n",  get_value(get_segment(1), 0));
  printf("Value: %s\n",  get_value(get_segment(1), 1));
 
  save_registry(get_segment(1), 1, "hello");
  printf("Pages[0]: %d\n", get_segment(1)->data.pages[0]);
  printf("Pages[1]: %d\n", get_segment(1)->data.pages[1]);
  
  printf("Value: %s\n",  get_value(get_segment(1), 0));
  printf("Value: %s\n",  get_value(get_segment(1), 1));
 
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
  int table_index = find_table(select_info->table_name);
  if(table_index != -1){
    segment* segment = get_segment(table_index);
    int page_index = find_page(segment, 3, select_info->key);
    if(page_index != -1){
      char* value = get_value(segment, page_index);
    }
  }
  else{

  }
  char* response = parse_package_select(select_info);
  send(fs_socket, response, strlen(response)+1, 0);


  return string_new("holis"); //tienen que devolver algo si no se rompe
}

char* action_insert(package_insert* insert_info){
  log_info(logger, "Se recibio una accion insert");
}

//en esta funcion se devuelve lo 
//proveniente del gossiping
//devuelve solo las seeds
//con esta forma: RETARDO_GOSSIPING_DE_ESTA_MEMORIA|id,ip,port|id,ip,port|id,ip,port
//                                                    seed        seed      seed
char* action_intern_memory_status(){
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
}

char* action_describe(package_describe* describe_info){
  log_info(logger, "Se recibio una accion describe");
}

char* action_drop(package_drop* drop_info){
  log_info(logger, "Se recibio una accion drop");
}

char* action_journal(package_journal* journal_info){
  log_info(logger, "Se recibio una accion select");
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
