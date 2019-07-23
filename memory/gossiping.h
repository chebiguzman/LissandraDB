#include <stdlib.h>
#include <commons/log.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include "../pharser.h"
#include "../actions.h"
#include "../console.h"

typedef struct gossip{
  int number;
  struct gossip* next;
  struct gossip* prev;
}gossip_t;
