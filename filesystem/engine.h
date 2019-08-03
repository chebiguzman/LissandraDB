#ifndef ENGINE_H
#define ENGINE_H

#include <commons/log.h> //logger
#include <stdlib.h>
#include <commons/log.h>
#include <pthread.h>
#include <string.h>
#include <commons/config.h>
#include <sys/stat.h> //creacion de directorios
#include <sys/types.h> //creacion de directorios
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include "../pharser.h"
#include <unistd.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include "../kernel/kmemory.h"

typedef struct {
    t_log* logger;
} fs_structure_info;

typedef struct{
    int partition_number;
    t_consistency consistency;
    long compactation_time;
}t_table_metadata;

typedef struct{
    char** blocks;
    int blocks_size;
}t_table_partiton;

typedef struct{

    char* name;
    int compactating;
    long compactation_time;
    pthread_mutex_t lock;
    pthread_cond_t *cond;
}t_table;

typedef struct{
  char* value;
  uint16_t timestap;
}row;


void* setup_fs(void* args);
void engine_start(t_log* logger);
int enginet_create_table(char* table_name, int consistency, int particiones, long compactation_time);
 int does_table_exist(char* table_name);
char* get_table_metadata_as_string(char* table_name);
char* get_all_tables_metadata_as_string();
t_table_metadata* get_table_metadata(char* table_name);
t_table_partiton* get_table_partition(char* table_name, int table_partition_number);
t_table_partiton* get_table_partition2(char* table_name, int table_partition_number);
t_table_partiton* get_table_partition2(char* table_name, int table_partition_number);
void engine_drop_table(char* table_name);

void engine_dump_table(char* table_name, char* table_dump);
int find_free_block();
void set_block_as_occupied(int block_number);
void set_block_as_free(int block_number);
int does_file_exist(char* file_path);
int find_tmp_name(char* tmp_path);
char* add_block_to_list(char* block_list,int );
void adjust_size(char* size,int tam);
void new_block(char* new_row,char* tabla,int particion);
void engine_compactate(char* table_name);
int contadordetemp(DIR* directorio);
 int contadordetempc(DIR* directorio);
t_table_partiton* particion_xd_parte1();
void particiontemporal();
int max_row_amount();
void engine_adjust(char* tabla,int particion,int adjust);
long get_dump_time();
void update_engine_config();
long get_retardo_time();
void* config_worker(void* args);
row* select_particiones_temporales(package_select* select_info);
t_table_partiton* get_table_partition3(char* table_name, int table_partition_number);
#endif /* ENGINE_H */