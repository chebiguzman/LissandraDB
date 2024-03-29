#include "../pharser.h"
typedef struct{
     char* line;
     int dirty;
    } regg;


typedef struct{
  int keey;
  char value [20];
} readrow;

typedef struct {
  char* ruta;
  int key;
  char* retorno;
  char* row;
  uint16_t timestap;
  char value [100];
  int bolean;
  pthread_cond_t* cond;
  pthread_mutex_t lock;
  int* number_of_running_threads;
  int *l;

}argumentosthread;

typedef struct {
  char* ruta;
  int key;
  char* retorno;
  char* row;
  char* new_row;
  int bolean;
  int hecho;
  pthread_cond_t* cond;
  pthread_mutex_t lock;
  int* number_of_running_threads;
  char* tabla;
  int part;

}argumentosthread_compactacion;

typedef struct {
  char* ruta;
  int key;
  long timestap_max;
  char* retorno;
  char* row;
  char value [100];
  int bolean;
  pthread_cond_t* cond;
  pthread_mutex_t lock;
  int* number_of_running_threads;
  

}argumentosthread2;



void obtengovalue(char* row, char* value);
void* buscador(void* args);
void* buscador2(void* args);
char *strdups(const char *src);
int contarbloques(char*);
void vaciarvector(char* puntero);
void leerarchivo(FILE* metadata, regg* regmetadata);
void obtenerbloques(char* pointer1, int* pointer2);
void vaciadobuffer(char* buffer);
void cortador(char* cortado, char* auxkey);
void* dump_cron(void* TIEMPO_DUMP);
char* MNT_POINT;
int get_size(char* linea);
int partition_num(char* numero);
int get_all_rows(char* ruta,regg* temp_rows,int block_number);
void reubicar_rows(regg* temp_rows,char* tabla,int reg_amount);
int contar_rows(char* ruta);
void adjust_size(char* size,int new_row);
void* buscador_compactacion(void* args);
void exec_err_abort();
void particiontemporal();