#include "segments.h"

page_t* create_page(int timestamp, int key, char* value){
	//TODO: LEVANTAR EXCEPCION SI EL VALUE ES MUY GRANDE????
	page_t* page = (page_t*)malloc(sizeof(page_t));
	page->timestamp = timestamp;
	page->key = key;
	strcpy(page->value, value);
	return page;
}

page_info_t* create_page_info(int dirty_bit){
  page_info_t* page_info = (page_info_t*)malloc(sizeof(page_info_t));
	page_info->next = NULL;
	page_info->dirty_bit = dirty_bit == 0 ? 0 : 1;
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

// guarda una pagina en memoria sin dirtybit porque es un select de fs
void save_page(segment_t* segment, page_t* page){ 
	page_info_t* page_info = find_page_info(segment, page->key);
	// si ya existe no hago nada, para modificar una pagina hay que usar el insert_page
	if(page_info == NULL){
		save_page_to_memory(segment, page, 0);
	}
}

// Agrega una nueva pagina o modifica una a existente siempre con dirtybit
void insert_page(segment_t* segment, page_t* page){
	page_info_t* page_info = find_page_info(segment, page->key);
	// si ya existe la pagina, reemplazo el value y toco el dirtybit
	if(page_info != NULL){
		if(page_info->page_ptr->timestamp < page->timestamp){ // si por alguna razon de la vida el timestamp del insert es menor al timestamp que ya tengo en la page, no la modifico
			memcpy(page_info->page_ptr->value, page->value, sizeof(VALUE_SIZE));
			page_info->dirty_bit = 1;
		}
	}
	// si no existe, creo una nueva con dirtybit (si no tiene dirtybit no se la mando a fs en el journaling)
	else{
		save_page_to_memory(segment, page, 1);
	}
}

// crea una page_info con o sin dirtybit, se la asigna al segmento, y guarda la pagina en main memory
void save_page_to_memory(segment_t* segment, page_t* page, int dirty_bit){ 
	// si no existe la pagina, busco una pagina libre en memoria y le guardo la page
	page_info_t* page_info = create_page_info(dirty_bit);
	int index = find_free_page();
	if(index == -1){
		return; // TODO: TIRAR ERROR
	}
	page_info->page_ptr = MAIN_MEMORY+index;
	memcpy(page_info->page_ptr, page, sizeof(page_t));
	add_page_to_segment(segment, page_info);
	update_LRU(page_info);
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

page_info_t* find_page_info(segment_t* segment, int key){
	page_info_t* temp = segment->pages;
	page_t* page;
	while(temp != NULL){
		if(temp->page_ptr->key == key){
			update_LRU(temp);
			return temp;
		}
		temp = temp->next;
	}
	return temp; // si no encontre nada hasta ahora, devuelvo temp que es NULL
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
	if(page != NULL){
  	printf("timestamp: %d, key:%d, value: %s\n", page->timestamp, page->key, page->value);
	}
	else {
		printf("page doesnt exist");
	}
}

LRU_TABLE_t* create_LRU_TABLE(){
	LRU_TABLE_t* lru = (LRU_TABLE_t*)malloc(sizeof(LRU_TABLE_t));
	lru->current_pages = 0;
	lru->lru_pages = (page_info_t**)malloc(sizeof(page_info_t*) * NUMBER_OF_PAGES);
	return lru;
}

// busca una pagina dentro del LRU TABLE y devuelve su index, sino -1
int find_page_in_LRU(page_info_t* page_info){
	page_info_t** lru_page_info;
	for(int i = 0; i < LRU_TABLE->current_pages; i++){
		lru_page_info = LRU_TABLE->lru_pages+i;
		printf("Comparando pages info ptrs: %p vs %p\n", *lru_page_info, page_info);			
		if(*lru_page_info == page_info){
			printf("Page found on index: %d\n", i);
			return i;
		}
	}
	return -1;
}

void update_LRU(page_info_t* page_info){
	int index = find_page_in_LRU(page_info);
	int last_page_index = LRU_TABLE->current_pages-1;
	printf("Lst page index: %d\n", last_page_index);
	if(index != -1){ // si ya esta en la tabla, la muevo al final
		memmove(LRU_TABLE->lru_pages+index, LRU_TABLE->lru_pages+index+1, NUMBER_OF_PAGES-1-index * sizeof(page_info_t*));
		memcpy(LRU_TABLE->lru_pages+last_page_index, &page_info, sizeof(page_info_t*));
	}
	else{ // si no esta en la tabla la agrego
		page_info_t* temp = page_info;
		memcpy(LRU_TABLE->lru_pages+last_page_index+1, &temp, sizeof(page_info_t*));
		LRU_TABLE->current_pages++;
		printf("LRUTABLE update: adding -%s- to the table\n", page_info->page_ptr->value);
	}
	print_LRU_TABLE();
}

void print_LRU_TABLE(){
	page_info_t** lru_page_info;
	printf("Pages in LRU Table (%d): \n", LRU_TABLE->current_pages);
	for(int i = 0; i < LRU_TABLE->current_pages; i++){
		lru_page_info = LRU_TABLE->lru_pages+i;
  	printf("%s ", (*lru_page_info)->page_ptr->value);
	}
	printf("\n");
}