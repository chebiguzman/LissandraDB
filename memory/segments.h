#include <stdlib.h>
#include <commons/log.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

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
  int index;
  struct page_info* next;
  struct page_info* prev;
}page_info_t;

typedef struct segment{
  char* name;
  page_info_t* pages;
  struct segment* next;
  struct segment* prev;
}segment_t;

typedef struct lru_page{
  page_info_t* lru_page;
  segment_t* segment;
}lru_page_t;

typedef struct{
  lru_page_t* lru_pages;
  int* used_pages;
  int current_pages;
}LRU_TABLE_t;


// ---- GLOBAL VARIABLES ----

segment_t* SEGMENT_TABLE;
page_t* MAIN_MEMORY; 
LRU_TABLE_t* LRU_TABLE;
int NUMBER_OF_PAGES;
t_log* logger;


// --------------------------


page_t* create_page(int timestamp, int key, char* value);
page_info_t* create_page_info();
segment_t* create_segment(char* table_names);
page_info_t* find_page_info(segment_t* segment, int key);
page_info_t* save_page(segment_t* segment, page_t* page);
page_info_t* insert_page(segment_t* segment, page_t* page);
void remove_from_segment(segment_t* segment, page_info_t* page_info);
page_info_t* save_page_to_memory(segment_t* segment, page_t* page, int dirtybit);
segment_t* find_segment(char* table_name);
segment_t* find_or_create_segment(char* table_name);
int find_free_page();
void remove_page(lru_page_t* lru_page_info);
void add_segment_to_table(segment_t* segment);
void add_page_to_segment(segment_t* segment, page_info_t* page_info);
segment_t* get_last_segment();
page_info_t* get_last_page(page_info_t* page_info);
void print_page(page_info_t* page_info);
void print_segment_table();
void print_segment_pages(segment_t* segment);
int find_page_in_LRU(page_info_t* page);
void update_LRU(segment_t* segment, page_info_t* page_info);
void remove_from_LRU(lru_page_t* lru_page_info);
void print_LRU_TABLE();
lru_page_t* create_lru_page(segment_t* segment, page_info_t* page_info);
LRU_TABLE_t* create_LRU_TABLE();
int is_modified(lru_page_t* page);
char* exec_in_memory(int memory_fd, char* payload);
void remove_segment(char* table_name);
int* get_used_pages();
int* update_used_pages();
int page_is_on_use(int index);
int find_unmodified_page();
char* exec_in_memory(int memory_fd, char* payload);

