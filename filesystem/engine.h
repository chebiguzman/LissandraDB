#include <commons/log.h> //logger

void* setup_fs(void* args);
void engine_start(t_log* logger);
typedef struct {
    t_log* logger;
} fs_structure_info;