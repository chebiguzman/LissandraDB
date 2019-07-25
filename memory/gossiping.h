#include "../server.h"
#include <commons/config.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include "segments.h"



typedef struct gossip{
  int number;
  struct gossip* next;
  struct gossip* prev;
}gossip_t;

// ---- GLOBAL VARIABLES ----
gossip_t* gossip_table;
// --------------------------

gossip_t* create_node();
void add_node(gossip_t* list, gossip_t* node);
void remove_node(gossip_t* list, gossip_t* temp);
gossip_t* create_gossip_table();
gossip_t* parse_gossip_buffer(int* buffer);
int* create_gossip_buffer(gossip_t* list);
gossip_t* find_node(gossip_t* list, int number);
gossip_t* compare_lists(gossip_t* list1, gossip_t* list2);
void gossip();