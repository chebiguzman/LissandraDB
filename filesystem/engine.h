#include <commons/log.h> //logger

void* setup_fs(void* args);
void engine_start();
typedef struct {
    t_log* logger;
} fs_structure_info;