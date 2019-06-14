#include <stdlib.h>
#include <commons/log.h>
#include <pthread.h>
#include <string.h>

// TODO: hacer el handshake y obtener el valor real
#define VALUE_SIZE 64
#define TABLE_NAME_SIZE 3

typedef struct{
  int timestamp;
  int key;
  char value[VALUE_SIZE];
}page_t;

typedef struct page_info{
  u_int8_t dirty_bit;
  page_t* page_ptr;
  struct page_info* next;
}page_info_t;

typedef struct segment{
  char* name;
  page_info_t* pages;
  struct segment* next;
}segment_t;

typedef struct{
  page_info_t** lru_pages;
  int current_pages;
}LRU_TABLE_t;


// ---- GLOBAL VARIABLES ----

segment_t* SEGMENT_TABLE;
page_t* MAIN_MEMORY; 
LRU_TABLE_t* LRU_TABLE;
int NUMBER_OF_PAGES;

// --------------------------


page_t* create_page(int timestamp, int key, char* value);
page_info_t* create_page_info();
segment_t* create_segment(char* table_names);
page_info_t* find_page_info(segment_t* segment, int key);
void save_page(segment_t* segment, page_t* page);
void insert_page(segment_t* segment, page_t* page);
void save_page_to_memory(segment_t* segment, page_t* page, int dirtybit);
segment_t* find_segment(char* table_name);
segment_t* find_or_create_segment(char* table_name);
int find_free_page();
void add_segment_to_table(segment_t* segment);
void add_page_to_segment(segment_t* segment, page_info_t* page_info);
segment_t* get_last_segment(segment_t* SEGMENT_TABLE);
page_info_t* get_last_page(page_info_t* page_info);
void print_page(page_t* page);
int find_page_in_LRU(page_info_t* page);
void update_LRU(page_info_t* page_info);
void print_LRU_TABLE();
LRU_TABLE_t* create_LRU_TABLE();