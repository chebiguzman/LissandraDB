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
	page_info->dirty_bit = dirty_bit == 0 ? 0 : 1;
	page_info->next = NULL;
	page_info->prev = NULL;
  	return page_info;
}

segment_t* create_segment(char* table_name){
  	segment_t* segment = (segment_t*)malloc(sizeof(segment_t));
	segment->name = table_name;
  	segment->pages = NULL;
	segment->next = NULL;
	segment->prev = NULL;
	add_segment_to_table(segment);
  	return segment;
}

int find_free_page(){
    page_t* page;
    for(int i = 0; i < NUMBER_OF_PAGES; i++){
		// TODO: fijarse directamente si esta en la lru
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
page_info_t* save_page(segment_t* segment, page_t* page){ 
	page_info_t* page_info = find_page_info(segment, page->key);
	
	// si ya existe no hago nada, para modificar una pagina hay que usar el insert_page
	if(page_info == NULL){
		page_info = save_page_to_memory(segment, page, 0);
	}
	return page_info;
}

// Agrega una nueva pagina o modifica una a existente siempre con dirtybit
page_info_t* insert_page(segment_t* segment, page_t* page){
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
	return page_info;
}

// crea una page_info con o sin dirtybit, se la asigna al segmento, y guarda la pagina en main memory
page_info_t* save_page_to_memory(segment_t* segment, page_t* page, int dirty_bit){ 
	// si no existe la pagina, busco una pagina libre en memoria y le guardo la page
	page_info_t* page_info = create_page_info(dirty_bit);
	int index = find_free_page();
	if(index == -1){
		return NULL; // TODO: TIRAR ERROR
	}
	page_info->page_ptr = MAIN_MEMORY+index;
	memcpy(page_info->page_ptr, page, sizeof(page_t));
	add_page_to_segment(segment, page_info);
	update_LRU(segment, page_info);
	return page_info;
}

void remove_from_segment(segment_t* segment, page_info_t* page_info){
	page_info_t* temp;
	if(page_info->prev == NULL){ // si es la primer page del segmento..
		printf("-- Removing %s (first node) from %s --\n", page_info->page_ptr->value, segment->name);
		temp = NULL; // si es el unico elemento, solo asigno segment->pages a temp que es NULL
		if(page_info->next != NULL){ 
			temp = page_info->next;
			temp->prev = NULL;
		}
		segment->pages = temp;
	}
	else {
		printf("-- Removing %s from %s --\n", page_info->page_ptr->value, segment->name);		
		temp = page_info->prev;
		temp->next = page_info->next;
	}
}

void remove_page(lru_page_t* lru_page_info){
	if(!is_modified(lru_page_info)){
		// remove_from_LRU(lru_page_info);
		printf("-- Removing %s from %s --\n", lru_page_info->lru_page->page_ptr->value, lru_page_info->segment->name);		
		remove_from_segment(lru_page_info->segment, lru_page_info->lru_page);
		// sacar de memoria (setearla a 0);
	}
}

int is_modified(lru_page_t* page){
	return page->lru_page->dirty_bit == 0 ? 0 : 1;
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
			update_LRU(segment, temp);
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
		segment->prev = NULL;
	}
	else{
		segment_t* temp = get_last_segment();
		temp->next = segment;
		segment->prev = temp;
	}
	print_segment_table();
}

void add_page_to_segment(segment_t* segment, page_info_t* page_info){
	if(segment->pages == NULL){
		segment->pages = page_info;
		page_info->prev = NULL;
	}
	else{
		page_info_t* temp = get_last_page(segment->pages);
		temp->next = page_info;
		page_info->prev = temp;
	}
}

segment_t* get_last_segment(){
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

void print_page(page_info_t* page_info){
	if(page_info != NULL){
		printf(" -- Value %s --\n Direccion: %p, Prev: %p, Next: %p\n", page_info->page_ptr->value ,page_info, page_info->prev, page_info->next);
		
  		// printf("timestamp: %d, key:%d, value: %s, dirty bit: %d\n", page_info->page_ptr->timestamp, page_info->page_ptr->key, page_info->page_ptr->value, page_info->dirty_bit);
	}
	else {
		printf("page doesnt exist");
	}
}

void print_segment_table(){
	segment_t* temp = SEGMENT_TABLE;
	printf("Segment Table: \n");
	while(temp != NULL){
		printf("%s ", temp->name);
		temp = temp->next;		
	}
	printf("\n\n");
}

void print_segment_pages(segment_t* segment){
	page_info_t* temp = segment->pages;
	printf("%s pages (%p):\n", segment->name, segment->pages);
	while(temp != NULL){
		print_page(temp);
		temp = temp->next;
	}
	printf("\n");
}

LRU_TABLE_t* create_LRU_TABLE(){
	LRU_TABLE_t* lru = (LRU_TABLE_t*)malloc(sizeof(LRU_TABLE_t));
	lru->current_pages = 0;
	lru->lru_pages = (lru_page_t*)malloc(sizeof(lru_page_t) * NUMBER_OF_PAGES);
	return lru;
}

// busca una pagina dentro del LRU TABLE y devuelve su index, sino -1
int find_page_in_LRU(page_info_t* page_info){
	lru_page_t* lru_page_info;
	for(int i = 0; i < LRU_TABLE->current_pages; i++){
		lru_page_info = LRU_TABLE->lru_pages+i;
		if(lru_page_info->lru_page == page_info){
			return i;
		}
	}
	return -1;
}

lru_page_t* create_lru_page(segment_t* segment, page_info_t* page_info){
	lru_page_t* lru_page_info = (lru_page_t*)malloc(sizeof(lru_page_t));
	lru_page_info->lru_page = page_info;
	lru_page_info->segment = segment;
	printf("Creating page -> value: %s, segment: %p\n", lru_page_info->lru_page->page_ptr->value, lru_page_info->segment);
	return lru_page_info;
}

void update_LRU(segment_t* segment, page_info_t* page_info){
	printf("-- Updating LRU --\n");	
	int index = find_page_in_LRU(page_info);
	int last_index = LRU_TABLE->current_pages-1;
	if(index != -1){ // si ya esta en la tabla, la muevo al final
		lru_page_t temp = *(LRU_TABLE->lru_pages+index);		
		memmove(LRU_TABLE->lru_pages+index, LRU_TABLE->lru_pages+index+1, NUMBER_OF_PAGES-1-index * sizeof(page_info_t));		
		memcpy(LRU_TABLE->lru_pages+last_index, &temp, sizeof(lru_page_t));
	}
	else{ // si no esta en la tabla la agrego
		lru_page_t* temp = create_lru_page(segment, page_info);
		memcpy(LRU_TABLE->lru_pages+last_index+1, temp, sizeof(lru_page_t));
		LRU_TABLE->current_pages++;
	}
	print_LRU_TABLE();
}

void remove_from_LRU(lru_page_t* lru_page_info){
	int index = find_page_in_LRU(lru_page_info->lru_page);
	printf("-- Removing %s from LRU (index: %d) --\n", lru_page_info->lru_page->page_ptr->value, index);	
	if(index != -1){
		memmove(LRU_TABLE->lru_pages+index, LRU_TABLE->lru_pages+index+1, NUMBER_OF_PAGES-1-index * sizeof(page_info_t*));
		LRU_TABLE->current_pages--;
	}	
}

void print_LRU_TABLE(){
	lru_page_t* lru_page_info;
	printf("Pages in LRU Table (%d): \n", LRU_TABLE->current_pages);
	for(int i = 0; i < LRU_TABLE->current_pages; i++){
		lru_page_info = LRU_TABLE->lru_pages+i;
  		printf("%s(%p) ", lru_page_info->lru_page->page_ptr->value, lru_page_info->segment);
	}
	printf("\n\n");
}

char* exec_in_memory(int memory_fd, char* payload){
    char* responce = malloc(3000);
    strcpy(responce, "");
    
    if ( memory_fd < 0 ){
      log_error(logger, "No se pudo llevar a cabo la accion.");
      return "";
    }

    //ejecutar
    if(write(memory_fd,payload, strlen(payload)+1)){
      read(memory_fd, responce, 3000);
      return responce;
    }else{
      log_error(logger, "No se logo comuniarse con FS");
      return "NO SE ENCUENTRA FS";
    }  
    return "algo sale mal";
}