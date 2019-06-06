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

typedef enum { EVENTUAL_CONSISTENCY, STRONG_CONSISTENCY, STRONG_HASH_CONSISTENCY } consistency_type ;

//SE ESTAN MOVIENDO LAS ESTRUCTURAS A PHARSER.c
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

