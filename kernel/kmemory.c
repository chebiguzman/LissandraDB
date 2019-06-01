#include "kmemory.h"
#include "scheduler.h"
#include <commons/collections/list.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "list_helper.h"
t_log* logger;
t_list* mem_list;
t_list* tbl_list;
t_list_helper* mem_list_helper;

pthread_mutex_t mem_list_lock;
int memory_count = 0;


typedef struct{
    char* name;
    t_consistency consistency;
}t_table;

void start_kmemory_module(t_log* logg, char* main_memory_ip, int main_memoy_port){
    logger = logg;
    mem_list = list_create();
    mem_list_helper = list_helper_init(mem_list);

    t_kmemory* mp = malloc(sizeof(t_kmemory));
    
    mp->id = 0;
    mp->fd = connect_to_memory(main_memory_ip, main_memoy_port);
    pthread_mutex_init(&mp->lock,NULL);

    list_add(mem_list, mp);

    log_info(logger, "El modulo kmemory fue inicializado exitosamente");

}

int get_loked_memory(t_consistency consistency){
    if(consistency = S_CONSISTENCY){
        log_debug(logger, "se pide memoria de SC");
        return getStrongConsistencyMemory();
    }else if( consistency == H_CONSISTENCY){
        log_debug(logger, "se pide memoria de HC");
    }else{
        log_debug(logger, "se pide memoria de EVENTUAL");
    }
}

int getStrongConsistencyMemory(){
    pthread_mutex_lock(&mem_list_lock);
    t_kmemory* mem = list_get(mem_list, 0);
    pthread_mutex_unlock(&mem_list_lock);
    pthread_mutex_lock(&mem->lock);
    return mem->fd;
}


void unlock_memory(int memoryfd){
    pthread_mutex_lock(&mem_list_lock);
    t_kmemory* mem = list_helper_find_memory_by_fd(mem_list_helper, memoryfd);
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
    
    ksyscall("DESCRIBE");
}