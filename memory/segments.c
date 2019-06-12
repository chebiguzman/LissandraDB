#include "segments.h"

page_t* create_page(int timestamp, int key, char* value){
	//TODO: LEVANTAR EXCEPCION SI EL VALUE ES MUY GRANDE????
	page_t* page = (page_t*)malloc(sizeof(page_t));
	page->timestamp = timestamp;
	page->key = key;
	strcpy(page->value, value);
	return page;
}

page_info_t* create_page_info(){
  page_info_t* page_info = (page_info_t*)malloc(sizeof(page_info_t));
	page_info->next = NULL;
  return page_info;
}

segment_t* create_segment2(char* table_names){
  segment_t* segment = (segment_t*)malloc(sizeof(segment_t));
  segment->pages = NULL;
  segment->next = NULL;
  return segment;
}

int find_free_page2(){
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

void save_page(segment_t* segment, page_t* page){ // TODO: reemplazar el value por la estructura que pase FS de key y value
		page_info_t* page_info = create_page_info();
    int index = find_free_page2();
		if(index == -1){
			return; // TODO: TIRAR ERROR
		}
		page_info->page_ptr = MAIN_MEMORY+index;
		memcpy(page_info->page_ptr, page, sizeof(page_t));
		add_page_to_segment(segment, page_info);
}

page_t* find_page2(segment_t* segment, int key){
	page_info_t* temp = segment->pages;
	page_t* page;
	while(temp != NULL){
		page = temp->page_ptr;
		if(temp->page_ptr->key == key){
			return page;
		}
		temp = temp->next;
	}
	return page; // si no encontre nada hasta ahora, devuelvo temp que es NULL
}

segment_t* find_or_create_segment(char* table_name){
	segment_t* segment = find_segment(table_name);
	if(segment != NULL){
		return segment;
	}
	segment = create_segment2(table_name);
	return segment;
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

segment_t* get_last_segment(segment_t* SEGMENT_TABLE2){
  segment_t* temp = SEGMENT_TABLE2;
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

void add_segment_to_table2(segment_t* segment){
	if(SEGMENT_TABLE2 == NULL){
		SEGMENT_TABLE2 = segment;
	}
	else{
		segment_t* temp = get_last_segment(SEGMENT_TABLE2);
		segment->next = temp->next;
		temp->next = segment;
	}
}

segment_t* find_segment(char* table_name){
	segment_t* temp = SEGMENT_TABLE2;
	while(temp != NULL){
		if(strcmp(temp->name, table_name) == 0){
			return temp;
		}
		temp = temp->next;
	}
	return temp; // si no encontre nada hasta ahora, devuelvo temp que es NULL
}