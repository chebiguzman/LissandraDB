#include "../pharser.h"
typedef struct{
     char* line;
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
  char value [100];
  int bolean;
  pthread_cond_t* cond;
  pthread_mutex_t lock;
  int* number_of_running_threads;

}argumentosthread;

void obtengovalue(char* row, char* value);
void* buscador(void* args);
char *strdups(const char *src);
int contarbloques(char*);
void vaciarvector(char* puntero);
void leerarchivo(FILE* metadata, regg* regmetadata);
void obtenerbloques(char* pointer1, int* pointer2);
void vaciadobuffer(char* buffer);
void cortador(char* cortado, char* auxkey);
char* MNT_POINT;
