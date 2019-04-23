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
#define INSTRUCTION_BYTE_SIZE 9 //tambien cargar en el pharser

void* create_server (void* args);

typedef unsigned int pakage_header;
typedef enum { EVENTUAL_CONSISTENCY, STRONG_CONSISTENCY, STRONG_HASH_CONSISTENCY } consistency_type ;
//SELECT [NOMBRE_TABLA] [KEY]
typedef struct package_select
{
    char* instruction;
    char* table_name;
    int key;

}package_select ;

//INSERT [NOMBRE_TABLA] [KEY] “[VALUE]”
typedef struct 
{
    char instruction[INSTRUCTION_BYTE_SIZE];
    char* table_name;
    int key;
    char* value;

} package_insert;

//CREATE [TABLA] [TIPO_CONSISTENCIA] [NUMERO_PARTICIONES] [COMPACTION_TIME]
typedef struct 
{
    char instruction[INSTRUCTION_BYTE_SIZE];
    char* table_name;
    consistency_type consistency;
    int partition_number;
    long compactation_time;

} package_create;

//DESCRIBE [NOMBRE_TABLA]
typedef struct
{
    char instruction[INSTRUCTION_BYTE_SIZE];
    char* table_name;

} package_describe;


//DROP [NOMBRE_TABLA]
typedef struct
{
    char instruction[INSTRUCTION_BYTE_SIZE];
    char* table_name;

} package_drop;

//JOURNAL
typedef struct
{
    char instruction[INSTRUCTION_BYTE_SIZE];

} package_journal;

//ADD MEMORY [NÚMERO] TO [CRITERIO]
typedef struct
{
    char instruction[INSTRUCTION_BYTE_SIZE];
    char memory[7];
    int memory_number;
    char to[3];
    consistency_type criterio;

} package_add;

//RUN <path>
typedef struct
{
    char instruction[INSTRUCTION_BYTE_SIZE];
    char* path;

} package_run;

//METRICS
typedef struct
{
    char instruction[INSTRUCTION_BYTE_SIZE];

} package_metrics;


//SERVER
typedef struct {
    int portNumber;
    t_log* logger;
} server_info;
#endif

