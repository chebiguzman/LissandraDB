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


void insert_to_memtable();
bool is_data_on_memtable(char* table_name, int key);
char* get_value_from_memtable(char* table_name, int key);
void dump_memtable();
char* dump_table(struct table_node* table);
char* get_row_from_memtable(char* table_name, int key);