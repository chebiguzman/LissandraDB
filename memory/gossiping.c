#include "gossiping.h"


gossip_t* create_node(int number){
    gossip_t* node = (gossip_t*)malloc(sizeof(gossip_t));
    node->next = NULL;
    node->prev = NULL;
    node->number = number;
    // printf("Creando nodo %d\n", number);
    return node;    
}

void add_node(gossip_t** gossip_table, gossip_t* node){
    // printf("Agregando nodo %d a la tabla... ", node->number);
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
    printf("- Removing node %d\n", node->number);
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

gossip_t* find_node(gossip_t** gossip_table, int number){
	gossip_t* temp = *gossip_table;
	while(temp != NULL){
		if(temp->number == number){
			return temp;
		}
		temp = temp->next;
	}
	return temp; // si no encontre nada en el loop, devuelvo temp que es NULL
}

// crea la gossip table del otro nodo con el buffer que le pasan
gossip_t* parse_gossip_buffer(gossip_t** gossip_table, char* buffer){
    printf("Parseando buffer: %s\n", buffer);
    char* temp_buffer = malloc(strlen(buffer) + 1);
    strcpy(temp_buffer, buffer);
    // temp_buffer = temp_buffer+(strlen("GOSSIP "));
    int number_size = 5;
    char* string_number = malloc(number_size+1);
    memcpy(string_number, temp_buffer, number_size);
    string_number[number_size] = 0;
    int size = atoi(string_number);
    for(int i = 1; i < size+1; i++){
        memcpy(string_number, temp_buffer+(number_size * i), number_size);
        gossip_t* node = create_node(atoi(string_number));
        add_node(gossip_table, node);
    }
    free(temp_buffer);
    return *gossip_table;
}

// retorna un string de 6 caracteres, 5 para el numero y el \0
// si tiene menos de 5 digitos, rellena el principio con 0s
char* itoa_for_buffer(char* str, int num){
    int i, max_size = 5, rem, len = 1, n;
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
    return 0;
}

// buffer para mandar entre nodos, cada buffer esta compuesto por un numero al inicio con la cantidad de puertos 
// que contiene el buffer, seguido de los numeros de los puertos . Cada numero (inclutendo el primero son de 5 chars)
char* create_gossip_buffer(gossip_t** gossip_table){
    int gossip_table_size = get_gossip_table_size(gossip_table);
    int number_size = 5;
    int buffer_size = number_size * (gossip_table_size + 1) + 1; // +1 para el \0
    gossip_t* temp = *gossip_table;
    char* buffer = (char*)malloc(buffer_size);
    char* string_number = malloc(6);
    itoa_for_buffer(string_number, gossip_table_size);
    strcpy(buffer, string_number);
    for(int i = 1; i < gossip_table_size+1; i++){
        itoa_for_buffer(string_number, temp->number);
        strcat(buffer, string_number);
        temp = temp->next;
    }
    free(string_number);
    return buffer;
}

gossip_t* compare_gossip_tables(gossip_t** gossip_table1, gossip_t** gossip_table2){
    printf("COMPARANDO TABLAS\n");
    gossip_t* compared_node = *gossip_table2;
    // gossip_t* temp2;
    // busca cada nodo de la tabla2 en la tabla1, si no lo encuentra, lo agrega
    while(compared_node != NULL){
        if(!find_node(gossip_table1, compared_node->number)){
            printf("No se encontro el nodo %d... Agregandolo\n", compared_node->number);
            add_node(gossip_table1, create_node(compared_node->number)); // creo uno nuevo, porque despues libero el puntero y lo pierdo sino
        }
        compared_node = compared_node->next;
    }
    printf("Asi quedo la ");
    print_gossip_table(gossip_table1);

    // busco cada nodo de gp1 en gp2, si no esta, lo saco de gp1 porque quiere decir que se desconecto el nodo
    // while(temp1 != NULL){
    //     temp2 = find_node(gossip_table2, temp1->number);
    //     if(temp2 == NULL){
    //         remove_node(gossip_table1, temp1);
    //     }
    //     temp1 = temp1->next; 
    // }

    // temp2 = *gossip_table2;
    // while(temp2 != NULL){
    //     temp1 = find_node(gossip_table1, temp2->number);
    //     if(temp1 == NULL){
    //         gossip_t* new_node = create_node(temp2->number);
    //         // add_node(new_node, temp1);
    //     }
    //     temp2 = temp2->next;
    // }
}

void print_gossip_table(gossip_t** gossip_table){
	gossip_t* temp = *gossip_table;
	printf("Gossip table: ");
	while(temp != NULL){
		printf("%d ", temp->number);
		temp = temp->next;		
	}
	printf("\n\n");
}

void* gossip(gossip_t** gossip_table, char* seed_port){
    printf("Gossiping...\n");
    // es mejor tener un array de char porque el ultimo elemento va a ser NULL y no me como un segfault cuando itero
    char** seed_ports = config_get_array_value(config, "PUERTO_SEEDS");
    memcpy(seed_ports[0], seed_port, 4); // ---- TODO: BORRAR
    char** seed_ips = config_get_array_value(config, "IP_SEEDS");
    int retardo_gossiping = config_get_int_value(config, "RETARDO_GOSSIPING") / 1000;
    // TODO: armar un array con los nodos del config + los de la gossip table menos el este nodo
    // while(1){
        for(int i=0; seed_ports[i] != NULL; i++){
            int seed_port = atoi(seed_ports[i]);
            printf("Connecting with node: %s...\n", seed_ports[i]);

            // setup client para conectarse con otro nodo   
            int seed_socket = socket(AF_INET, SOCK_STREAM, 0);
            char* next_ip = "127.0.0.1";
            printf("%s %d\n", next_ip, seed_port);
            struct sockaddr_in sock_client;
            
            sock_client.sin_family = AF_INET; 
            sock_client.sin_addr.s_addr = inet_addr(next_ip); 
            sock_client.sin_port = htons(seed_port);

            int connection_result =  connect(seed_socket, (struct sockaddr*)&sock_client, sizeof(sock_client));
            
            if(connection_result < 0){
                log_error(logger, "No se logro establecer la conexion con el siguiente nodo");   
                // TODO: hay que borrar el nodo con el que no conecto? o todo y volver a empezar el gossiping
                remove_node(gossip_table, find_node(gossip_table, seed_port));
            }

            else{
                char* instruction = "GOSSIP ";
                char* string_gossip_table = create_gossip_buffer(gossip_table);
                char* gossip_buffer = malloc(strlen(instruction) + strlen(string_gossip_table) + 1);
                strcpy(gossip_buffer, instruction);
                strcat(gossip_buffer, string_gossip_table);

                char* response = malloc(500);
                if(write(seed_socket, gossip_buffer, strlen(gossip_buffer) + 1)){
                    read(seed_socket, response, 499);
                    log_info(logger, "Se logro conectar con el siguiente nodo");
                    gossip_t* temp_gossip = NULL;
                    parse_gossip_buffer(&temp_gossip, response);

                    compare_gossip_tables(gossip_table, &temp_gossip);
                    // liberar tabla????
                    free(response);
                }
                else{
                  remove_node(gossip_table, find_node(gossip_table, seed_port));   
                }
            }
        // }
        sleep(retardo_gossiping);
    }

    // si falla, borrar la memoria de la gossip_tablea actual 
    // si se conecta, comparar las gossip tales y cambiar de ambas
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