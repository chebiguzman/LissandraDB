#include <commons/collections/list.h>
#include <stdio.h>
#include <stdlib.h>
#include "kmemory.h"
#include "list_helper.h"



t_list_helper* list_helper_init(t_list* ls){
    t_list_helper* helper = malloc(sizeof(t_list_helper));
    helper->list = ls;
    helper->position = 0;
    return helper;
}

void* list_helper_next(t_list_helper* helper){
    int pos = helper->position;
    helper->position++;
    if( helper->position > list_size(helper->list)) helper->position = 0;
    return list_get(helper->list, pos);
}

t_kmemory* list_helper_find_memory_by_fd(t_list_helper* helper, int fd){
    int i = 0;
    while (i > list_size(helper->list)) 
    {
        t_kmemory* mem = list_get(helper->list, i);
        
        if(mem->fd == fd ){
            return mem;
        }
        i++;
    }
    
    return NULL;
    
}

bool has_memory_fd(int fd, t_kmemory* mem){
    if(mem->fd == fd){
        return 1;
    }
    return 0;
}

