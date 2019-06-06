#include <stdlib.h>
#include <commons/log.h>
#include <pthread.h>
#include <string.h>
#include <commons/config.h>
#include "memtable.h"
#include <errno.h>
#include "../pharser.h"

//struct table_node* memtable_first = NULL;
//TODO ACA TIENE QUE USAR SEMAFOTOS
struct table_node* memtable_p;

void insert_to_memtable(package_insert* insert_info) {

    //Porque tenes copia de los structs???
    
    if (memtable_p == NULL) { //la memtable esta vacia - agrego tabla nueva y cargo la data
        
        printf("La mem table eesta vacia\n");
        
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

        /*free(new_nodo_table);
        free(new_nodo_data);*/ //sensillamente no, es deshacer todo lo que acabas de hacer

    } else { //la memtable no esta vacia - busco si existe la tabla
        struct table_node* aux_table;
        aux_table = memtable_p;
            printf("voy a buscar una tabla\n");

        while(aux_table->table_next != NULL) { //si la tabla en la que estoy parado no es
            printf("encontre una tabla de nombre: %s\n",aux_table->table_name );
            
            if(!strcmp(aux_table->table_name,insert_info->table_name)){
                break;
            } 
            aux_table = aux_table->table_next;

        }
            printf("salgo del while: %s\n",aux_table->table_name );

        if (!strcmp(aux_table->table_name, insert_info->table_name)) { //Si encontro la tabla - agrego la data ... asi no se comparan strings
            struct data_node* aux_data;
            aux_data = aux_table->data;
            printf("con los datos:\n");

            while (aux_data->data_next != NULL) { //Avanzo hasta el ultimo nodo de data
                printf("%s\n", aux_data->value);
                aux_data = aux_data->data_next; //no deberiamos aca descartar los varios  timestamps?
                

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
        printf("no encontre la tabla, la inserto al final \n");

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