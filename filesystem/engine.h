#include <commons/log.h> //logger


void* setup_fs(void* args);
void engine_start(t_log* logger);
int enginet_create_table(char* table_name, int consistency, int particiones, long compactation_time);
 int does_table_exist(char* table_name);

typedef struct {
    t_log* logger;
} fs_structure_info;