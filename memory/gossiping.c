#include "gossiping.h"

gossip_t* create_node(){
    gossip_t* node = (gossip_t*)malloc(sizeof(gossip_t));
    node->next = NULL;
    node->prev = NULL;
    return node;    
}

void add_node(gossip_t* list, gossip_t* node){
	if(list == NULL){ // si esta vacia
		list = node;
	}
	else{
		gossip_t* temp = list;
        while(temp->next != NULL){
            temp = temp->next; // itero hasta llegar al utlimo nodo
        }
		temp->next = node;
		node->prev = temp;
	}
}

void remove_node(gossip_t* list, gossip_t* temp){
    printf("- Removing node %d\n", temp->number);
	if(temp->next != NULL){ // si no es el ultimo..
		temp->next->prev = temp->prev; // le asigno al siguiente de temp, su anterior, o null en caso de que sea el primero
	}
	if(temp->prev != NULL){ // si no es el primero..
		temp->prev->next = temp->next;
	}
	else{ // en caso de que sea el primero..
		list = temp->next;
	}
}

gossip_t* parse_gossip_buffer(int* buffer){
    gossip_t* gossip_table = NULL;
    int size = *buffer+0;
    buffer = buffer+sizeof(int);
    for(int i = 0; i < size; i++){
        gossip_t* node = create_node();
        node->number = *buffer+i+1;
        add_node(gossip_table, node);
    }
    return gossip_table;
}

int* create_gossip_buffer(gossip_t* list){
    int list_size = 0;
    gossip_t* temp = list;
    while(temp != NULL){
        list_size++;
        temp = temp->next;
    }
    temp = list;
    int* buffer = (int*)malloc(sizeof(int) * (list_size + 1));
    int n = list_size;
    memcpy(buffer+0, &n, sizeof(int));
    for(int i = 1; i < list_size+1; i++){
        memcpy(buffer+i, &temp->number, sizeof(int));
        temp = temp->next;
    }
    return buffer;
}

gossip_t* find_node(gossip_t* list, int number){
	gossip_t* temp = list;
	while(temp != NULL){
		if(temp->number == number){
			return temp;
		}
		temp = temp->next;
	}
	return temp; // si no encontre nada en el loop, devuelvo temp que es NULL
}

gossip_t* compare_lists(gossip_t* list1, gossip_t* list2){
    gossip_t* temp1 = list1;
    gossip_t* temp2;
    int number;
    while(temp1 != NULL){
        number = temp1->number;
        temp2 = find_node(list2, number);
        if(temp2 == NULL){
            remove_node(list1, temp1);
        }
        temp1 = temp1->next; 
    }

    temp2 = list2;
    while(temp2 != NULL){
        number = temp2->number;
        temp1 = find_node(list1, number);
        if(temp1 == NULL){
            gossip_t* new_node = create_node();
            new_node->number = number;
            add_node(new_node, temp1);
        }
        temp2 = temp2->next;
    }
}

