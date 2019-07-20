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
#include <commons/collections/dictionary.h>
#include "../pharser.h"
#include "../actions.h"
#include "../console.h"
#include <commons/string.h>
#include <commons/collections/queue.h>
#include <signal.h>

//punto de entrada para el programa y el kernel
t_log* logger;
t_log* logger_debug;
char* MEMORY_IP;
int MEMORY_PORT;
int main(int argc, char const *argv[])
{   
   //inicio lectura por consola
    pthread_t tid_console;
    pthread_create(&tid_console, NULL, console_input, "test");
    
    pthread_join(tid_console,NULL);
    
    //FREE MEMORY
    //free(logger);
    //free(serverInfo);

      return 0;
}


char* action_select(package_select* select_info){
  char* package = parse_package_select(select_info);
  return package;

}

char* action_run(package_run* run_info){
    return "";
}

char* action_add(package_add* add_info){
  return "";
}

char* action_insert(package_insert* insert_info){
  char* package = parse_package_insert(insert_info);
  return package;
}

char* action_create(package_create* create_info){
  char* package = parse_package_create(create_info);
  return package;
}

char* action_describe(package_describe* describe_info){
  char* package = parse_package_describe(describe_info);
  return package;
}

char* action_drop(package_drop* drop_info){
  char* package = parse_package_drop(drop_info);
  return package;
}

char* action_journal(package_journal* journal_info){
  char* package = parse_package_journal(journal_info);
  return package;
}

char* action_metrics(package_metrics* metrics_info){
  log_info(logger_debug, "Se recibio una accion metrics");
  return "";
}

char* action_intern__status(){
    return "";
}

//Crea un t_instr con un string
char* parse_input(char* input){
  return exec_instr(input);
}
