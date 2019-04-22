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
typedef struct package_select
{
    char header[HEADER_BYTE_SIZE];
    char* table_name;
    int key;

}package_select ;

//INSERT [NOMBRE_TABLA] [KEY] “[VALUE]”
typedef struct 
{
    char* header[HEADER_BYTE_SIZE];
    char table_name;
    int key;
    char* value;

} package_insert;

//CREATE [TABLA] [TIPO_CONSISTENCIA] [NUMERO_PARTICIONES] [COMPACTION_TIME]
typedef struct 
{
    char* header[HEADER_BYTE_SIZE];
    char* table_name;
    char* consistency;
    int partition_number;
    long compactation_time;

} package_create;

//DESCRIBE [NOMBRE_TABLA]
typedef struct
{
    char* header[HEADER_BYTE_SIZE];
    char* table_name;

} package_describe;


//DROP [NOMBRE_TABLA]
typedef struct
{
    char* header[HEADER_BYTE_SIZE];
    char* table_name;

} package_drop;

//JOURNAL
typedef struct
{
    char* header[HEADER_BYTE_SIZE];

} package_journal;

//ADD MEMORY [NÚMERO] TO [CRITERIO]
typedef struct
{
    char* header[HEADER_BYTE_SIZE];
    char memory[6];
    int memory_number;
    char to[2];
    char* criterio;

} package_add;

//RUN <path>
typedef struct
{
    char* header[HEADER_BYTE_SIZE];
    char* path;

} package_run;

//METRICS
typedef struct
{
    char* header[HEADER_BYTE_SIZE];

} package_metrics;


//SERVER
typedef struct {
    int portNumber;
    t_log* logger;
} server_info;
#endif

