#ifndef SERVER_H_   
#define SERVER_H_

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> //sock structures
#include <commons/log.h> //loggger
#include <commons/string.h> //string append
#include <unistd.h> //read function
#define HEADER_BYTE_SIZE 9 //tambien cargar en el pharser
void* create_server (void* args);

//SELECT [NOMBRE_TABLA] [KEY]
typedef struct pakage_select
{
    char header[HEADER_BYTE_SIZE];
    char* table_name;
    int key;

}pakage_select ;

//INSERT [NOMBRE_TABLA] [KEY] “[VALUE]”
typedef struct 
{
    char* header[HEADER_BYTE_SIZE];
    char table_name;
    int key;
    char* value;
} pakage_insert;

//CREATE [TABLA] [TIPO_CONSISTENCIA] [NUMERO_PARTICIONES] [COMPACTION_TIME]
typedef struct 
{
    char* header[HEADER_BYTE_SIZE];
    char* table_name;
    char* consistency;
    int partition_number;
    long compactation_time;

} create_pakage;


//DESCRIBE [NOMBRE_TABLA]
//DROP [NOMBRE_TABLA]
//JOURNAL
//ADD MEMORY [NÚMERO] TO [CRITERIO]
//RUN <path>
//METRICS

typedef struct {
    int portNumber;
    t_log* logger;
} server_info;
#endif

