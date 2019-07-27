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
gossip_t* GOSSIP_TABLE;
int PORT;
// --------------------------

gossip_t* create_node(int number);
void add_node(gossip_t** gossip_table, gossip_t* node);
void remove_node(gossip_t** gossip_table, gossip_t* node);
gossip_t* find_node(gossip_t** gossip_table, int number);
gossip_t* create_gossip_table();
gossip_t* parse_gossip_buffer(int* buffer);
char* create_gossip_buffer(gossip_t** gossip_table);
gossip_t* find_node(gossip_t** gossip_table, int number);
gossip_t* compare_lists(gossip_t* gossip_table1, gossip_t* gossip_table2);
void print_gossip_table();
void* gossip();
int get_gossip_table_size(gossip_t** gossip_table);
char* tostring(char str[], int num);