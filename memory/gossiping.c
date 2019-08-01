#include "gossiping.h"

int length_indicator = 2, length_number = 5;

gossip_t* create_node(int port, char* ip){
    gossip_t* node = (gossip_t*)malloc(sizeof(gossip_t));
    node->next = NULL;
    node->prev = NULL;
    node->port = port;
    node->ip = strdup(ip);
    // printf("Creando nodo %d\n", port);
    return node;    
}

void add_node(gossip_t** gossip_table, gossip_t* node){
    // printf("Agregando nodo %d a la tabla... ", node->port);
    if(find_node(gossip_table, node->port)) return;
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
    // printf("Nodo agregado.\n");    
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
}

gossip_t* find_node(gossip_t** gossip_table, int port){
	gossip_t* temp = *gossip_table;
	while(temp != NULL){
		if(temp->port == port){
			return temp;
		}
		temp = temp->next;
	}
	return temp; // si no encontre nada en el loop, devuelvo temp que es NULL
}

// crea la gossip table del otro nodo con el buffer que le pasan
gossip_t* parse_gossip_buffer(char* buffer){
    // printf("Parseando: %s\n", buffer);
    gossip_t* new_gossip_table = NULL;
    char* temp_buffer = strdup(buffer);
    // printf("Buffer asd(length: %d): %s\n", strlen(buffer), buffer);
    printf("Buffer(length: %d): %s\n", strlen(temp_buffer), temp_buffer);
    char* string_number_of_nodes = strndup(temp_buffer, length_indicator);
    int number_of_nodes = atoi(string_number_of_nodes);
    temp_buffer = temp_buffer+length_indicator;
    printf("Parsed number of nodes: %d\n", number_of_nodes);
    
    char* string_port;
    // char* string_number = strndup(buffer, length_number);
    char* string_ip_length;
    char* new_ip;
    // temp_buffer = temp_buffer+length_number;
    // while(temp_buffer != 0){
    for(int i = 0; i < number_of_nodes; i++){
        // sleep(1);
        //leo el port (5 chars)
        string_port = strndup(temp_buffer, length_number);
        int new_port = atoi(string_port);
        temp_buffer = temp_buffer+length_number;
        // printf("Parsed port: %d\n", new_port);

        // sleep(1);
        //leo el length indicator (2 chars)
        string_ip_length = strndup(temp_buffer, length_indicator);
        int ip_length = atoi(string_ip_length);
        temp_buffer = temp_buffer+length_indicator;
        // printf("Parsed ip length: %d\n", ip_length);
        
        // sleep(1);
        //leo el ip (<ip_length> chars)
        new_ip = strndup(temp_buffer, ip_length);
        temp_buffer = temp_buffer+ip_length;
        // printf("Parsed ip: %s\n", new_ip);
        
        // temp_buffer = temp_buffer+strlen(new_ip);        
        // memcpy(string_number, temp_buffer+(length_number * i), length_number);
        gossip_t* node = create_node(new_port, new_ip);
        add_node(&new_gossip_table, node);
        print_gossip_table(&node);
    }
    // free(temp_buffer);
    return new_gossip_table;
}

// retorna un string de 6 caracteres, 5 para el numero y el \0
// si tiene menos de 5 digitos, rellena el principio con 0s
char* itoa_for_buffer(char* str, int max_size, int num){
    int i, rem, len = 1, n;
    n = num / 10;
    while (n != 0){
        len++;
        n /= 10;
    }
    for (i = 0; i < max_size; i++){
        if(i > len){
            str[max_size - (i + 1)] = '0';
        }else{
            rem = num % 10;
            num = num / 10;
            str[max_size - (i + 1)] = rem + '0';
        }
    }
    str[max_size] = '\0'; // le agrego el \0 al final para terminar el string
    return strdup(str);
}

// buffer para mandar entre nodos, cada buffer esta compuesto por un numero al inicio con la cantidad infos de nodos 
// que contiene el buffer, seguido de la info de nodos
// Cada info de nodo tiene 5 chars para el puerto, 2 chars para la length del ip, y el ip
char* create_gossip_buffer(gossip_t** gossip_table){
    // char sep[2] = { ',', '\0' };
    // char div[2] = { '|', '\0' };
    printf("Creando Buffer \n");
    int gossip_table_size = get_gossip_table_size(gossip_table);
    gossip_t* temp = *gossip_table;
    char* buffer = (char*)malloc(1000);
    char* string_number_of_nodes = malloc(length_indicator + 1);
    itoa_for_buffer(string_number_of_nodes, length_indicator, gossip_table_size);
    strcpy(buffer, string_number_of_nodes);
  
    while(temp != NULL){
        char* string_port = malloc(length_number + 1);
        itoa_for_buffer(string_port, length_number, temp->port);
        strcat(buffer, string_port);
        // printf("String port: %s\n", string_port);

        char* string_ip_length = malloc(length_indicator + 1);
        itoa_for_buffer(string_ip_length, length_indicator, strlen(temp->ip));
        strcat(buffer, string_ip_length);
        // printf("IP length: %s\n", string_ip_length);

        strcat(buffer, temp->ip);
        // printf("IP: %s\n", temp->ip);
        temp = temp->next;
    }
    char* short_buffer = strdup(buffer);
    free(buffer);
    printf("Buffer generado: %s \n", short_buffer);
    return short_buffer;
}

// busca cada nodo de la tabla2 en la tabla1, si no lo encuentra, lo agrega
void compare_gossip_tables(gossip_t** gossip_table1, gossip_t** gossip_table2){
    gossip_t* gossip_temp = *gossip_table2;
    while(gossip_temp != NULL){
        gossip_t* temp_node = create_node(gossip_temp->port, gossip_temp->ip);
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
    while(temp_gossip2 != NULL){
        gossip_t* temp_node = create_node(temp_gossip2->port, temp_gossip2->ip);
        add_node(&temp_gossip, temp_node);
        temp_gossip2 = temp_gossip2->next;
    }
    remove_node(&temp_gossip, temp_gossip);
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
        
        printf("Gossiping...\n");
        gossip_t* nodes_to_connect = create_nodes_to_connect(gossip_table, seeds_ports);
        printf("Gossip table posta: ");
        print_gossip_table(gossip_table);
        
        printf("Nodes to connnect with: ");
        print_gossip_table(&nodes_to_connect);
        while(nodes_to_connect != NULL){
            int seed_port = nodes_to_connect->port;
            printf("Connecting with node: %d...\n", seed_port);

            // setup client para conectarse con otro nodo   
            int seed_socket = socket(AF_INET, SOCK_STREAM, 0);
            char* next_ip = "127.0.0.1";
            struct sockaddr_in sock_client;
            
            sock_client.sin_family = AF_INET; 
            sock_client.sin_addr.s_addr = inet_addr(next_ip); 
            sock_client.sin_port = htons(seed_port);

            int connection_result =  connect(seed_socket, (struct sockaddr*)&sock_client, sizeof(sock_client));
            
            if(connection_result < 0){
                log_error(logger, "No se logro establecer la conexion con el siguiente nodo");   
                remove_node(gossip_table, find_node(gossip_table, seed_port));
            }

            else{
                char* instruction = "GOSSIP ";
                char* string_gossip_table = create_gossip_buffer(gossip_table);
                char* gossip_buffer = malloc(strlen(instruction) + strlen(string_gossip_table) + 1);
                strcpy(gossip_buffer, instruction);
                strcat(gossip_buffer, string_gossip_table);
                printf("Me conecte con el nodo y le mando esta tabla: ");
                print_gossip_table(gossip_table);

                char* response = malloc(500);
                if(write(seed_socket, gossip_buffer, strlen(gossip_buffer) + 1)){
                    read(seed_socket, response, 499);
                    printf("Buffer recibido: %s\n", response);
                    gossip_t* gossip_temp = parse_gossip_buffer(response);
                    compare_gossip_tables(gossip_table, &gossip_temp);
                    printf("Me conecto con una memoria y actualizo la ");
                    print_gossip_table(gossip_table);
                    // liberar tabla????
                    free(response);
                }

            }
            nodes_to_connect = nodes_to_connect->next;
        }
        printf("Despues de gossip: ");
        print_gossip_table(gossip_table);
        config_save(config);
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