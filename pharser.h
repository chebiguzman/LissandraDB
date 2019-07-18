#include "server.h"
#include "kernel/kmemory.h"



//SELECT [NOMBRE_TABLA] [KEY]
typedef struct {
    char* instruction;
    char* table_name;
    int key;

}package_select ;

//RUN <path>
typedef struct
{
    char* instruction;
    char* path;

} package_run;

//INSERT [NOMBRE_TABLA] [KEY] "[VALUE]" [TIMESTAMP]
typedef struct {
    char* instruction;
    char* table_name;
    int key;
    char* value;
    unsigned long timestamp;
} package_insert;

//DESCRIBE [NOMBRE_TABLA]?
typedef struct {
    char* instruction;
    char* table_name;
} package_describe;

//DROP [NOMBRE_TABLA]
typedef struct
{
    char* instruction;
    char* table_name;

} package_drop;


//ADD MEMORY [NÚMERO] TO [CRITERIO]
typedef struct 
{
    char* instruction;
    int id;
    t_consistency consistency;
}package_add;

//CREATE [TABLA] [TIPO_CONSISTENCIA] [NUMERO_PARTICIONES] [COMPACTION_TIME]
typedef struct 
{
    char* instruction;
    char* table_name;
    t_consistency consistency;
    int partition_number;
    long compactation_time;

} package_create;

char* exec_instr(char* input);
char* create_buffer(int argc, char const *argv[]);
char* get_string_from_buffer(char* buffer, int index);

char* parse_package_insert(package_insert* package);
char* parse_package_select(package_select* package);
char* parse_package_describe(package_describe* package);
char* parse_package_drop(package_drop* package);
char* parse_package_create(package_create* package);
char* parse_package_run(package_run* package);
char* parse_input(char* input);

