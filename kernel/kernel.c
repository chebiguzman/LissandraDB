#include <stdlib.h>
#include <commons/log.h>
#include "../server.h"
char* LOGPATH = "kernel.log";
//punto de entrada para el programa y el kernel
int main(int argc, char const *argv[])
{
    //TODO levantar archivo de configuracion para saber path del log

    t_log* logger;
    logger = log_create(LOGPATH, "Kernel", 1, LOG_LEVEL_INFO);
    serverThread(8080,NULL, logger);


      return 0;
}
