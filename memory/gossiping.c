#include "gossiping.h"

char g_sep[2] = { ',', '\0' };
char g_div[2] = { '|', '\0' };

gossip_t* create_node(int port, char* ip){
    gossip_t* node = (gossip_t*)malloc(sizeof(gossip_t));
    node->next = NULL;
    node->prev = NULL;
    node->port = port;
    node->ip = strdup(ip);
    // free(ip);
    return node;    
}

// ((((LIBERA EL NODO SI NO LO PUEDE AGREGAR)))))
void add_node(gossip_t** gossip_table, gossip_t* node){
    // printf("Agregando nodo %d a la tabla... ", node->port);
    if(find_node(gossip_table, node->port, node->ip)){
        free_node(node);
        return;
    }
	if(*gossip_table == NULL){ // si esta vacia
		*gossip_table = node;
	}
	else{
		gossip_t* temp = *gossip_table;
        while(temp->next != NULL){
            temp = temp->next; // itero hasta llegar al utlimo nodo
        }
		temp->next = node;
		node->prev = temp;
	}
}

void free_node(gossip_t* node){
    free(node->ip);
    free(node);
}

void remove_node(gossip_t** gossip_table, gossip_t* node){
    if(node == NULL) return;
    // printf("- Removing node %d\n", node->port);
	if(node->next != NULL){ // si no es el ultimo..
		node->next->prev = node->prev; // le asigno al siguiente de temp, su anterior, o null en caso de que sea el primero
	}
	if(node->prev != NULL){ // si no es el primero..
		node->prev->next = node->next;
	}
	else{ // en caso de que sea el primero..
		*gossip_table = node->next;
	}
    free_node(node);
}

void delete_table(gossip_t** gossip_table){
    gossip_t* temp = *gossip_table;
    gossip_t* temp2;
    while(temp != NULL){
        temp2 = temp->next;
        remove_node(gossip_table, temp);
        temp = temp2;
    }
    // free(*gossip_table);
}

gossip_t* find_node(gossip_t** gossip_table, int port, char* ip){
	gossip_t* temp = *gossip_table;
	while(temp != NULL){
		if(temp->port == port && !strcmp(ip, temp->ip)){
			return temp;
		}
		temp = temp->next;
	}
	return temp; // si no encontre nada en el loop, devuelvo temp que es NULL
}

int get_next_value_length(char* buffer){
    char* temp_buffer = strdup(buffer);
    int i = 0;
    while(temp_buffer[i] != g_sep[0] && temp_buffer[i] != g_div[0] && temp_buffer[i] != 0){
        i++;
    }
    free(temp_buffer);
    return i;
}

// crea la gossip table del otro nodo con el buffer que le pasan
gossip_t* parse_gossip_buffer(char* buffer){
    gossip_t* new_gossip_table = NULL;
    // char* temp_buffer = strdup(buffer);
    char* temp_buffer = malloc(1000);
    memset(temp_buffer, 0, 1000);
    strcpy(temp_buffer, buffer);
    char* temp_buffer_address = temp_buffer; // me tengo que guardar la address para liberarlo despues

    // printf("Char value: %d\n", temp_buffer[strlen(temp_buffer)+1]);
    int next_value_length;

    while(temp_buffer[0] != 0){
        next_value_length = get_next_value_length(temp_buffer);
        char* string_number = strndup(temp_buffer, next_value_length);
        int new_number = atoi(string_number);
        temp_buffer += next_value_length + 1;
        printf("Parsed number: %d\n", new_number);        

        next_value_length = get_next_value_length(temp_buffer);
        char* string_port = strndup(temp_buffer, next_value_length);
        int new_port = atoi(string_port);
        temp_buffer += next_value_length + 1;
        printf("Parsed port: %d\n", new_port);        

        next_value_length = get_next_value_length(temp_buffer);
        char* new_ip = strndup(temp_buffer, next_value_length);
        temp_buffer += next_value_length + 1;        
        printf("Parsed ip: %s\n", new_ip);

        gossip_t* node = create_node(new_port, new_ip);
        node->number = new_number;
        add_node(&new_gossip_table, node);
        // print_gossip_table(&node);

        free(string_number);
        free(string_port);
        free(new_ip);
    }
    free(temp_buffer_address);
    return new_gossip_table;
}

char* create_gossip_buffer(gossip_t** gossip_table){
    //printf("Creando Buffer \n");
    gossip_t* temp = *gossip_table;
    char* buffer = (char*)malloc(1000);
    memset(buffer, 0, 1000);
    char* string_number;
  
    while(temp != NULL){
        string_number = string_itoa(temp->number);
        strcat(buffer, string_number);
        strcat(buffer, g_sep);
        free(string_number);

        string_number = string_itoa(temp->port);        
        strcat(buffer, string_number);
        strcat(buffer, g_sep);
        free(string_number);        

        strcat(buffer, temp->ip);
        strcat(buffer, g_div);

        temp = temp->next;
    }
    char* short_buffer = strdup(buffer); // no se si sirve de algo, pero deberia ser puntero a un bloque de memoria mas chico
    free(buffer);
    // printf("Buffer generado: %s \n", short_buffer);
    return short_buffer;
}

// busca cada nodo de la tabla2 en la tabla1, si no lo encuentra, lo agrega
void compare_gossip_tables(gossip_t** gossip_table1, gossip_t** gossip_table2){
    gossip_t* gossip_temp = *gossip_table2;
    while(gossip_temp != NULL){
        gossip_t* temp_node = create_node(gossip_temp->port, gossip_temp->ip);
        temp_node->number = gossip_temp->number;
        add_node(gossip_table1, temp_node);
        gossip_temp = gossip_temp->next;
    }
}

void print_gossip_table(gossip_t** gossip_table){
	gossip_t* temp = *gossip_table;
	printf("Gossip table: ");
	while(temp != NULL){
		printf("%d ", temp->port);
		temp = temp->next;		
	}
	printf("\n");
}

// armo una gossip_table con los nodos que se tienen que contactar (los de la gossip table y los de config en caso de que se hayan sacado de la table)
gossip_t* create_nodes_to_connect(gossip_t** gossip_table, char** seeds_ports){
    gossip_t* temp_gossip = NULL;
    gossip_t* temp_gossip2 = *gossip_table;

    // copio la GOSSIP_TABLE en otra tabla
    while(temp_gossip2 != NULL){
        gossip_t* temp_node = create_node(temp_gossip2->port, temp_gossip2->ip);
        add_node(&temp_gossip, temp_node);
        temp_gossip2 = temp_gossip2->next;
    }
    
    // saco este nodo para que no se conecte con si mismo
    remove_node(&temp_gossip, temp_gossip);

    // creo y agrego los nodos del config (solo se agregan si no estan ya)
    for(int i = 0; seeds_ports[i] != NULL; i++){
        gossip_t* temp_node = create_node(atoi(seeds_ports[i]), seeds_ips[i]);
        add_node(&temp_gossip, temp_node);   
    }
    return temp_gossip;
}

void* gossip(void* void_gossip_table){
    gossip_t** gossip_table = (gossip_t**)void_gossip_table;
    while(1){
        pthread_mutex_lock(&gossip_table_mutex);					
        
        log_info(logger, "Iniciando Gossiping");
        gossip_t* nodes_to_connect = create_nodes_to_connect(gossip_table, seeds_ports);
        gossip_t* nodes_to_connect_address = nodes_to_connect; // me guardo el address del primer nodo para liberarlo despues
        // printf("Nodes to connect with: ");
        // print_gossip_table(&nodes_to_connect);
        while(nodes_to_connect != NULL){
            int seed_port = nodes_to_connect->port;
            char* seed_ip = nodes_to_connect->ip;
            log_info(logger, "Conectandose con el nodo %d...", seed_port);

            // setup client para conectarse con otro nodo   
            int seed_socket = socket(AF_INET, SOCK_STREAM, 0);
            struct sockaddr_in sock_client;
            
            sock_client.sin_family = AF_INET; 
            sock_client.sin_addr.s_addr = inet_addr(seed_ip); 
            sock_client.sin_port = htons(seed_port);

            int connection_result =  connect(seed_socket, (struct sockaddr*)&sock_client, sizeof(sock_client));
            
            // si no se conecta, lo saco de la gossip table
            if(connection_result < 0){
                log_error(logger, "No se pudo conectar con el nodo %d", seed_port);   
                remove_node(gossip_table, find_node(gossip_table, seed_port, seed_ip));
            }

            // si se conecta, le paso mi gossip table y el otro nodo me va a pasar su gossip table. La comparo con la mia y agrego los que faltan
            else{
                log_info(logger, "Se logro conectar con el nodo %d", seed_port); 
                char* instruction = "GOSSIP ";
                char* string_gossip_table = create_gossip_buffer(gossip_table);
                char* gossip_buffer = malloc(strlen(instruction) + strlen(string_gossip_table) + 1);
                strcpy(gossip_buffer, instruction);
                strcat(gossip_buffer, string_gossip_table);
                free(string_gossip_table);
                // printf("Me conecte con el nodo y le mando esta tabla: ");
                print_gossip_table(gossip_table);

                char* response = malloc(1000);
                if(write(seed_socket, gossip_buffer, strlen(gossip_buffer) + 1)){
                    read(seed_socket, response, 1000);
                    // printf("Buffer recibido: %s\n", response);
                    gossip_t* gossip_temp = parse_gossip_buffer(response);
                    log_info(logger, "Comparando las tablas gossip");
                    compare_gossip_tables(gossip_table, &gossip_temp);
                    // printf("Me conecto con una memoria y actualizo la ");
                    // print_gossip_table(gossip_table);
                    
                    delete_table(&gossip_temp);
                    free(gossip_buffer);
                    free(response);
                    close(seed_socket);
                }
            }
            nodes_to_connect = nodes_to_connect->next;
        }
        delete_table(&nodes_to_connect_address);
        log_info(logger, "Gossiping terminado");
        // printf("Despues de gossip: ");
        // print_gossip_table(gossip_table);
        // config_save(config);
        pthread_mutex_unlock(&gossip_table_mutex);					
        
        sleep(config_get_int_value(config, "RETARDO_GOSSIPING") / 1000);
    }
}

int get_gossip_table_size(gossip_t** gossip_table){
    int gossip_table_size = 0;
    gossip_t* temp = *gossip_table;
    while(temp != NULL){
        gossip_table_size++;
        temp = temp->next;
    }
    return gossip_table_size;
}