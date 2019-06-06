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

//ADD MEMORY [NÚMERO] TO [CRITERIO]
typedef struct 
{
    char* instruction;
    int id;
    t_consistency consistency;
}package_add;


char* exec_instr(char* input);
char* create_buffer(int argc, char const *argv[]);
char* get_string_from_buffer(char* buffer, int index);
char* get_value_from_buffer(char* buffer, int index);


char* parse_package_select(package_select* package);
char* parse_package_run(package_run* pk);

char* parse_package_insert(package_insert* package);
char* parse_package_describe(package_describe* package);

char* parse_input(char* input);

