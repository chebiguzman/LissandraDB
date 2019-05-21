#include <commons/log.h> //logger

void insert_memtable(void* args);

//Estructuras necesarias para la memtable:

struct data_node {
    unsigned timestamp;
    int key;
    char* value;
    struct data_node* data_next;
};

//t_data* data_last = NULL;

struct table_node{
    char* table_name;
    struct data_node* data;
    struct table_node* table_next;
};

//t_table* memtable_first = NULL;