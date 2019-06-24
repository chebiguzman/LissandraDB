#include <stdlib.h>
#include <commons/log.h>
#include <pthread.h>
#include <string.h>
#include <commons/config.h>
#include "memtable.h"
#include <errno.h>
#include "../pharser.h"
#include <string.h>

//struct table_node* memtable_first = NULL;
//TODO ACA TIENE QUE USAR SEMAFOTOS
struct table_node* memtable_p;

void insert_to_memtable(package_insert* insert_info) {
    
    if (memtable_p == NULL) { //la memtable esta vacia - agrego tabla nueva y cargo la data
        
        
        struct table_node* new_nodo_table;
        new_nodo_table = (struct table_node*)malloc(sizeof(struct table_node));
        new_nodo_table->table_next = NULL;
        new_nodo_table->table_name = insert_info->table_name;

        //cargo la data 
        struct data_node* new_nodo_data;
        new_nodo_data = (struct data_node*)malloc(sizeof(struct data_node));
        new_nodo_data->timestamp = insert_info->timestamp;
        new_nodo_data->key = insert_info->key;
        new_nodo_data->value = insert_info->value;
        new_nodo_data->data_next = NULL;

        //le asigno la data a la tabla
        new_nodo_table->data = new_nodo_data;

        //agrego la tabla a la memtable
        memtable_p = new_nodo_table;

    } else { //la memtable no esta vacia - busco si existe la tabla
        struct table_node* aux_table;
        aux_table = memtable_p;

        while(aux_table->table_next != NULL) { //si la tabla en la que estoy parado no es
            
            if(!strcmp(aux_table->table_name,insert_info->table_name)){
                break;
            } 
            aux_table = aux_table->table_next;

        }

        if (!strcmp(aux_table->table_name, insert_info->table_name)) { //Si encontro la tabla - agrego la data ... asi no se comparan strings
            struct data_node* aux_data;
            aux_data = aux_table->data;

            while (aux_data->data_next != NULL) { //Avanzo hasta el ultimo nodo de data
                aux_data = aux_data->data_next;
                
                if(aux_data->key == insert_info->key && aux_data->timestamp < insert_info->timestamp){
                    aux_data->timestamp = insert_info->timestamp;
                    aux_data->value = insert_info->value;
                    return;
                }

            }
            //cargo la data 
            struct data_node* new_nodo_data;
            new_nodo_data = (struct data_node*)malloc(sizeof(struct data_node));
            new_nodo_data->timestamp = insert_info->timestamp;
            new_nodo_data->key = insert_info->key;
            new_nodo_data->value = insert_info->value;
            new_nodo_data->data_next = NULL;
            aux_data->data_next = new_nodo_data;


        } else { //No encontro la tabla pero llego al final de la memtable - agrego la tabla y la data

        struct table_node* new_nodo_table;
        new_nodo_table = (struct table_node*)malloc(sizeof(struct table_node));
        new_nodo_table->table_next = NULL;
        new_nodo_table->table_name = insert_info->table_name;

        //cargo la data 
        struct data_node* new_nodo_data;
        new_nodo_data = (struct data_node*)malloc(sizeof(struct data_node));
        new_nodo_data->timestamp = insert_info->timestamp;
        new_nodo_data->key = insert_info->key;
        new_nodo_data->value = insert_info->value;
        new_nodo_data->data_next = NULL;

        //le asigno la data a la tabla
        new_nodo_table->data = new_nodo_data;

        //agrego la tabla a la memtable
        aux_table->table_next = new_nodo_table;

        }
    }

    return;
}

bool is_data_on_memtable(char* table_name, int key){
    struct table_node* aux_table;
    aux_table = memtable_p;
    while (aux_table!=NULL)
    {
        if(!strcmp(aux_table->table_name, table_name)){
        
            struct data_node* table_data;
            table_data = aux_table->data;

            while(table_data!=NULL){
                if(key == table_data->key) return true;
                table_data = table_data->data_next;
            }

        }

        aux_table = aux_table->table_next;
    }
    
    return false;
}

char* get_value_from_memtable(char* table_name, int key){
    struct table_node* aux_table;
    aux_table = memtable_p;
    while (aux_table!=NULL)
    {
        if(!strcmp(aux_table->table_name, table_name)){
        
            struct data_node* table_data;
            table_data = aux_table->data;

            while(table_data!=NULL){
                if(key == table_data->key) return table_data->value;
                table_data = table_data->data_next;
            }

        }

        aux_table = aux_table->table_next;
    }
    
    return false;
    return "#Error";
}