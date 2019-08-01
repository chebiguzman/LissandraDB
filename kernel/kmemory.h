#ifndef KMEMORY_H
#define KMEMORY_H

#include <pthread.h>
#include <commons/log.h>
#include <stdlib.h>
#include <commons/collections/dictionary.h>



typedef enum {
    S_CONSISTENCY,
    H_CONSISTENCY,
    ANY_CONSISTENCY,
    ERR_CONSISTENCY,
    ALL_CONSISTENCY
}t_consistency;

typedef struct{
    int id;
    int fd;
    t_consistency consistency;
    pthread_mutex_t lock;
}t_kmemory;

t_consistency get_table_consistency(char* table_name);
int get_loked_memory(t_consistency consistency, char* table_name);
void unlock_memory(int memoryfd);
void start_kmemory_module(t_log* logg,t_log* logg_debug, char* main_memory_ip, int main_memoy_port);
int connect_to_memory(char* ip, int port);
void *metadata_service(void* args);
void kmemory_set_active_tables(t_dictionary* dic);

int get_loked_main_memory();
void check_for_new_memory(char* ip, int port, int id);
void* memory_finder_service(void* args);
void update_memory_finder_service_time(long time);

void add_memory_to_sc(int id);
void add_memory_to_hc(int id);
void add_memory_to_any(int id);

int get_sc_memory();
int get_any_memory();
int get_hc_memory(char* table_name);
int get_memory();

int hash(char* string);
void kmemoy_add_table();
void kmemory_drop_table(char* tbl_name);
void kmemory_add_table(char* name, t_consistency* cons);
void disconect_from_memory(int memoryfd);
void set_main_memory_id(int id);

#endif /* KMEMORY_H */

