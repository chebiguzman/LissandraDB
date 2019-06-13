#include "segments.h"

page_t* create_page(int timestamp, int key, char* value){
	//TODO: LEVANTAR EXCEPCION SI EL VALUE ES MUY GRANDE????
	page_t* page = (page_t*)malloc(sizeof(page_t));
	page->timestamp = timestamp;
	page->key = key;
	strcpy(page->value, value);
	return page;
}

page_info_t* create_page_info(){//int dirty_bit){
  page_info_t* page_info = (page_info_t*)malloc(sizeof(page_info_t));
	page_info->next = NULL;
	// page_info->dirty_bit = dirty_bit == 0 ? 0 : 1;
  return page_info;
}

segment_t* create_segment(char* table_name){
  segment_t* segment = (segment_t*)malloc(sizeof(segment_t));
	segment->name = table_name;
  segment->pages = NULL;
  segment->next = NULL;
	add_segment_to_table(segment);
  return segment;
}

int find_free_page(){
    page_t* page;
    for(int i = 0; i < NUMBER_OF_PAGES; i++){
        page = MAIN_MEMORY+i;
        if(page->timestamp == 0){ // si no tiene timestamp quiere decir que no esta asignada (60% seguro de esto)
            return i;
        }
    }
    return -1;
    // TODO: algoritmo de reemplazo LRU
    // TODO: JOURNALING ATR
}

/* TODO: capaz conviene hacer que el save_page primero se fije si existe ya una page en ese segmento
con esa key, y si existe, la modifica y le cambia el dirty bit.
Estoy pensando en el caso cuando haya varias memorias, si dos threads por alguna razon hacen
el mismo select, una va a guardarlo en memoria antes que la otra. Si pasa esto vamos a guardar
dos keys iguales (creo)
*/
void save_page(segment_t* segment, page_t* page){//, int dirty_bit){ 
		page_info_t* page_info = create_page_info();//dirty_bit);
    int index = find_free_page();
		if(index == -1){
			return; // TODO: TIRAR ERROR
		}
		page_info->page_ptr = MAIN_MEMORY+index;
		memcpy(page_info->page_ptr, page, sizeof(page_t));
		add_page_to_segment(segment, page_info);
}

segment_t* find_segment(char* table_name){
	segment_t* temp = SEGMENT_TABLE;
	while(temp != NULL){
		if(strcmp(temp->name, table_name) == 0){
			return temp;
		}
		temp = temp->next;
	}
	return temp; // si no encontre nada hasta ahora, devuelvo temp que es NULL
}

page_t* find_page(segment_t* segment, int key){
	page_info_t* temp = segment->pages;
	page_t* page;
	while(temp != NULL){
		page = temp->page_ptr;
		if(temp->page_ptr->key == key){
			return page;
		}
		temp = temp->next;
	}
	return NULL; // si no encontre nada hasta ahora, devuelvo temp que es NULL
}

segment_t* find_or_create_segment(char* table_name){
	segment_t* segment = find_segment(table_name);
	if(segment != NULL){
		return segment;
	}
	segment = create_segment(table_name);
	return segment;
}

void add_segment_to_table(segment_t* segment){
	if(SEGMENT_TABLE == NULL){
		SEGMENT_TABLE = segment;
	}
	else{
		segment_t* temp = get_last_segment(SEGMENT_TABLE);
		temp->next = segment;
	}
}

void add_page_to_segment(segment_t* segment, page_info_t* page_info){
	if(segment->pages == NULL){
		segment->pages = page_info;
	}
	else{
		page_info_t* temp = get_last_page(segment->pages);
		temp->next = page_info;
	}
}

segment_t* get_last_segment(segment_t* SEGMENT_TABLE){
  segment_t* temp = SEGMENT_TABLE;
  while(temp->next != NULL){
    temp = temp->next;
  }
  return temp;
}

page_info_t* get_last_page(page_info_t* page_info){
  page_info_t* temp = page_info;
  while(temp->next != NULL){
    temp = temp->next;
  }
  return temp;
}

void print_page(page_t* page){
  printf("timestamp: %d, key:%d, value: %s\n", page->timestamp, page->key, page->value);
	
}