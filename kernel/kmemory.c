#include "kmemory.h"
#include "scheduler.h"
#include <commons/collections/list.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <unistd.h>
t_log* logger;
t_list* mem_list;   //Lista de memorias
t_list* tbl_list;   //Lista de tablas (metadata)
//t_list_helper* mem_list_helper;

pthread_mutex_t mem_list_lock;  //mutex para la lista de memorias
int memory_count = 0; //No se.. Borrar?
int strong_memory_pos = 0; //La memoria SC por default es la principal

typedef struct{
    char* name;
    t_consistency consistency;
}t_table;

void start_kmemory_module(t_log* logg, char* main_memory_ip, int main_memoy_port){
    logger = logg;
    mem_list = list_create();
    //mem_list_helper = list_helper_init(mem_list);

    t_kmemory* mp = malloc(sizeof(t_kmemory));
    
    mp->id = 0;
    mp->fd = connect_to_memory(main_memory_ip, main_memoy_port);
    pthread_mutex_init(&mp->lock,NULL);

    if(mp->id > -1){
        list_add(mem_list, mp);
    }else{
        log_error(logger, "No Existe memoria principal");
        strong_memory_pos = -1;
    }
 
    pthread_t tid_metadata_service;
    pthread_create(&tid_metadata_service, NULL,metadata_service, NULL);
    pthread_join(tid_metadata_service, NULL);

    log_info(logger, "El modulo kmemory fue inicializado exitosamente");

}

int get_loked_memory(t_consistency consistency){
    if(consistency == S_CONSISTENCY){
        log_debug(logger, "se pide memoria de SC");
        return getStrongConsistencyMemory();
    }else if( consistency == H_CONSISTENCY){
        log_debug(logger, "se pide memoria de HC");
    }else if (consistency == ANY_CONSISTENCY){
        log_debug(logger, "se pide memoria de EVENTUAL");
    }else{
        log_error(logger, "Fatal error. El sistema no reconoce el tipo de memoria solicitado");
        exit(-1);
    }
}

int getStrongConsistencyMemory(){

    if(strong_memory_pos == -1){
        log_error(logger, "No hay memoria en el citerio principal");
        return -1;
    }

    pthread_mutex_lock(&mem_list_lock);
    t_kmemory* mem = list_get(mem_list, strong_memory_pos);
    pthread_mutex_unlock(&mem_list_lock);
    pthread_mutex_lock(&mem->lock);
    return mem->fd;
}


void unlock_memory(int memoryfd){

    bool has_memory_fd(void* memory){
        t_kmemory* mem = memory;
       if(mem->fd == memoryfd){
           return true;
       }
       return false;
    }

    pthread_mutex_lock(&mem_list_lock);
    t_kmemory* mem = list_find(mem_list, has_memory_fd);
    pthread_mutex_unlock(&mem->lock);
    pthread_mutex_unlock(&mem_list_lock);
}



int connect_to_memory(char* ip, int port){

    int memoryfd = socket(AF_INET, SOCK_STREAM, 0); 
    struct sockaddr_in sock_client;
    sock_client.sin_family = AF_INET; 
    sock_client.sin_addr.s_addr = inet_addr( ip ); 
    sock_client.sin_port = htons( port );
    
    int conection_result = connect(memoryfd, (struct sockaddr*)&sock_client, sizeof(sock_client));

    if(conection_result<0){
      log_error(logger, "No se logro establecer coneccion con memoria");
      return -1;
    }
    return memoryfd;
}

void *metadata_service(void* args){
    while(true){
        char* r = ksyscall("DESCRIBE");
        log_debug(logger, r);
        //usleep(1000 * 1000);
    }
    
}

t_consistency get_table_consistency(char* table_name){
    return S_CONSISTENCY;
}