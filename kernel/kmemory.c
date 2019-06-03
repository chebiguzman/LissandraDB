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
extern scheduler_config* config_not;
extern pthread_mutex_t config_lock;

t_log* logger;
t_list* mem_list;   //Lista de memorias
t_list* tbl_list;   //Lista de tablas (metadata)

t_list* hc_list;
pthread_mutex_t any_lock = PTHREAD_MUTEX_INITIALIZER;
t_list* any_list;
//t_list_helper* mem_list_helper;

pthread_mutex_t mem_list_lock;  //mutex para la lista de memorias

pthread_mutex_t sc_lock = PTHREAD_MUTEX_INITIALIZER;
t_kmemory* strong_memory; //La memoria SC por default es la principal


typedef struct{
    char* name;
    t_consistency consistency;
}t_table;


void start_kmemory_module(t_log* logg, char* main_memory_ip, int main_memoy_port){
    logger = logg;
    mem_list = list_create();
    tbl_list = list_create();
    
    hc_list = list_create();
    any_list = list_create();

    t_table* table_debug = malloc(sizeof(t_table));

    //DEBUG
    table_debug->name = strdup("nombre");
    table_debug->consistency = S_CONSISTENCY;
    list_add(tbl_list,table_debug);
    //mem_list_helper = list_helper_init(mem_list);

    t_kmemory* mp = malloc(sizeof(t_kmemory));
    
    mp->id = 0;
    mp->fd = connect_to_memory(main_memory_ip, main_memoy_port);
    pthread_mutex_init(&mp->lock,NULL);

    if(mp->fd > -1){
        list_add(mem_list, mp);
        strong_memory = mp;
    }else{
        log_error(logger, "kmemory: No Existe memoria principal");
        strong_memory = NULL;
    }
 
    pthread_t tid_metadata_service;
    pthread_create(&tid_metadata_service, NULL,metadata_service, NULL);
    //pthread_join(tid_metadata_service, NULL);

    log_info(logger, "kmemory: El modulo kmemory fue inicializado exitosamente");

}

int get_loked_memory(t_consistency consistency){
    if(consistency == S_CONSISTENCY){
        log_debug(logger, "kmemory: se pide memoria de SC");
        return getStrongConsistencyMemory();
    }else if( consistency == H_CONSISTENCY){
        log_debug(logger, "kmemory: se pide memoria de HC");
    }else if (consistency == ANY_CONSISTENCY){
        log_debug(logger, "kmemory: se pide memoria de EVENTUAL");
    }else{
        log_error(logger, "Fatal error. Nombre de tabla equivocada o el sistema no reconoce el tipo de memoria solicitado");
        exit(-1);
    }
}

int getStrongConsistencyMemory(){

    if(strong_memory == NULL){
        log_error(logger, "kmemory: No hay memoria en el citerio principal");
        return -1;
    }

    log_debug(logger, "kmemory: bloqueo memoria");
    /*pthread_mutex_lock(&mem_list_lock);
    t_kmemory* mem = list_get(mem_list, strong_memory_pos);
    pthread_mutex_unlock(&mem_list_lock);
    pthread_mutex_lock(&mem->lock);*/
    pthread_mutex_lock(&strong_memory->lock);
    return strong_memory->fd;
}

void add_memory_to_sc(int id){
    printf("el id de la memoria es: %d\n", id);
    bool find_memory_by_id(void* m){
        printf("busco en la lista" );
        t_kmemory* mem = (t_kmemory*) m;
        pthread_mutex_lock(&mem->lock);
        int memId = mem->id;
        printf("busco en la lista. Esta memoria tiene un id:%d\n",memId );
        pthread_mutex_unlock(&mem->lock);
        if(memId == id){
            return true;
        printf("encontre la memoria!" );

        }
        return false;
    }
    t_kmemory* finded = list_find(mem_list, find_memory_by_id);

    if(finded == NULL){
        log_error(logger, "La memoria no existe. Revise las memorias e intentelo mas tarde");
        return;
    }

    pthread_mutex_lock(&sc_lock);
    strong_memory = finded;
    pthread_mutex_unlock(&sc_lock);

}   //TEST

void add_memory_to_any(int id){

    bool find_memory_by_id(void* m){
        t_kmemory* mem = m;
        pthread_mutex_lock(&mem->lock);
        int memId = mem->id;
        pthread_mutex_unlock(&mem->lock);
        if(memId == id){
            return true;
        }
        return false;
    }
    t_kmemory* finded = list_find(mem_list, find_memory_by_id);

    if(finded == NULL){
        log_error(logger, "La memoria no existe. Revise las memorias e intentelo mas tarde.");
        return;
    }

    t_kmemory* anys = list_find(any_list, find_memory_by_id);

    if(anys != NULL){
        log_error(logger, "La memoria ya se encuentra en el criterio.");
        return;
    }

    pthread_mutex_lock(&any_lock);
    list_add(any_list, finded);
    pthread_mutex_unlock(&any_lock);


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
    log_debug(logger, "kmemory: delbloqueo memoria");
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
      log_error(logger, "kmemory: No se logro establecer coneccion con memoria");
      return -1;
    }
    return memoryfd;
}

t_consistency get_table_consistency(char* table_name){

    bool find_table_by_name(void * t){
        t_table* tb = t;
        if( !strcmp(tb->name, table_name ) ){
            return true;
        }
        return false;
    }

    t_table* table = list_find(tbl_list, find_table_by_name );

    if(table == NULL){
        return ERR_CONSISTENCY;
    }
    return table->consistency;
}

void *metadata_service(void* args){
    while(true){
        pthread_mutex_lock(&config_lock);
        long sleep_interval = config_not->metadata_refresh * 1000;
         pthread_mutex_unlock(&config_lock);
        usleep( sleep_interval);
        log_debug(logger, "metadataService: Se actualiza la metadata de las tablas");
        char* r = ksyscall("DESCRIBE");
        log_debug(logger, r);
        
    }
    
}