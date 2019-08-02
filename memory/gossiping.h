#ifndef GOSSIPING_H
#define GOSSIPING_H

// #include "../server.h"
#include <commons/config.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
// #include "../actions.h"
#include "segments.h"


typedef struct gossip{
  int number;
  int port;
  char* ip;
  struct gossip* next;
  struct gossip* prev;
}gossip_t;

// ---- GLOBAL VARIABLES ----
gossip_t* GOSSIP_TABLE;
int MEMORY_PORT;
char* MEMORY_IP;
char** seeds_ports;
char** seeds_ips;
pthread_mutex_t gossip_table_mutex;
// --------------------------

gossip_t* create_node(int port, char* ip);
void add_node(gossip_t** gossip_table, gossip_t* node);
void remove_node(gossip_t** gossip_table, gossip_t* node);
gossip_t* find_node(gossip_t** gossip_table, int port, char* ip);
gossip_t* create_gossip_table();
gossip_t* parse_gossip_buffer(char* buffer);
char* create_gossip_buffer(gossip_t** gossip_table);
void compare_gossip_tables(gossip_t** gossip_table1, gossip_t** gossip_table2);
void print_gossip_table();
void* gossip(void* gossip_table);
int get_gossip_table_size(gossip_t** gossip_table);
char* itoa_for_buffer(char* str, int max_size, int num);
gossip_t* create_nodes_to_connect(gossip_t** gossip_table, char** seeds_ports);
int get_next_value_length(char* buffer);
void delete_table(gossip_t** gossip_table);

#endif