#include <stdlib.h>
#include <commons/log.h>
#include <pthread.h>
#include <string.h>

// TODO: hacer el handshake y obtener el valor real
#define VALUE_SIZE 64
#define TABLE_NAME_SIZE 32

typedef struct{
  char* name;
  char* base;
  int limit;
  int pages[3];
}segment_info;

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

typedef struct segmentt{
  char* name;
  page_info_t* pages;
  struct segmentt* next;
}segment_t;

typedef struct segment{
  segment_info data;
  struct segment *next;
}segment;

segment_t* SEGMENT_TABLE2;
page_t* MAIN_MEMORY;
int NUMBER_OF_PAGES;

page_t* create_page(int timestamp, int key, char* value);
page_info_t* create_page_info();
segment_t* create_segment2(char* table_names);
int find_free_page2();
void save_page(segment_t* segment, page_t* page);
void add_page_to_segment(segment_t* segment, page_info_t* page_info);
segment_t* get_last_segment(segment_t* SEGMENT_TABLE2);
page_info_t* get_last_page(page_info_t* page_info);
void add_segment_to_table2(segment_t* segment);
segment_t* find_segment(char* table_name);




segment* create_segment();
segment* get_segment(int index);
void add_segment_to_table(int index, segment* new_segment);
char* get_end_memory_address(int index);
char* get_first_memory_address_after(int index);
int find_memory_space(int memory_needed);
int save_segment_to_memory(segment_info segment_info);
void print_segment_info(int index);
int find_table(char* table_name);
int find_page(segment* segment, int size, int key);
int find_free_page(int pages[], int number_of_pages);
void add_key_to_table(segment* segment, int index, int key);
int get_memory_offset(char* base);
void free_memory_space(char* address, int size);
char* get_page_address(segment* segment, int page_index);
char* save_value_to_memory(segment* segment, int page_index, char* value);
void save_registry(segment* segment, int key, char* value);
char* get_value(segment* segment, int page_index);
