#include "server.h"

//SELECT [NOMBRE_TABLA] [KEY]
typedef struct package_select
{
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

char* parse_bytearray(char* buffer);
char* create_buffer(int argc, char const *argv[]);
char* get_string_from_buffer(char* buffer, int index);


char* parse_package_select(package_select* package);
char* parse_package_run(package_run* pk);

