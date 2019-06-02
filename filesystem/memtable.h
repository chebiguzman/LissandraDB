#include <commons/log.h> //logger

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


void insert_memtable(void* args, struct table_node* memtable_p);