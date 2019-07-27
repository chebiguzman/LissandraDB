#include "gossiping.h"


gossip_t* create_node(int number){
    gossip_t* node = (gossip_t*)malloc(sizeof(gossip_t));
    node->next = NULL;
    node->prev = NULL;
    node->number = number;
    return node;    
}

void add_node(gossip_t** gossip_table, gossip_t* node){
	if(*gossip_table == NULL){ // si esta vacia
        printf("Agregando nodo %d a la tabla.\n", node->number);
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

gossip_t* parse_gossip_buffer(int* buffer){
    gossip_t* gossip_table = NULL;
    int size = *buffer+0;
    buffer = buffer+sizeof(int);
    for(int i = 0; i < size; i++){
        gossip_t* node = create_node(*buffer+i+1);
        add_node(&gossip_table, node);
    }
    free(buffer);
    return gossip_table;
}

// crea un buffer con los primeros 4 chars indicando el tamaño, y los siguientes sets de 4 chars cada puerto
// capaz falta inclur el ip ademas del puerto
// char* create_gossip_buffer(gossip_t** gossip_table){
//     int gossip_table_size = get_gossip_table_size(gossip_table);
//     int port_size = sizeof(char) * 5; // 5 es la max cantidad de digitos para un puerto; 65535
//     gossip_t* temp = *gossip_table;
//     char* buffer = (char*)malloc((gossip_table_size + 1) + port_size + 1); 
//     strcat(buffer+0, &gossip_table_size);
//     for(int i = 1; i < gossip_table_size+1; i++){
//         memcpy(buffer+i, &temp->number, sizeof(int));
//         temp = temp->next;
//     }
//     return buffer;
// }

// retorna un string de 6 caracteres, 5 para el numero y el \0
// si tiene menos de 5 digitos, rellena el principio con 0s
char* tostring(char* str, int num){
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

// buffer con array de ints
// int* create_gossip_buffer(gossip_t** gossip_table){
//     int gossip_table_size = get_gossip_table_size(gossip_table);
//     gossip_t* temp = *gossip_table;
//     int* buffer = (int*)malloc(sizeof(int) * (gossip_table_size + 1));
//     memcpy(buffer+0, &gossip_table_size, sizeof(int));
//     for(int i = 1; i < gossip_table_size+1; i++){
//         memcpy(buffer+i, &temp->number, sizeof(int));
//         temp = temp->next;
//     }
//     return buffer;
// }

// buffer para mandar entre nodos, cada buffer esta compuesto por un numero al inicio con la cantidad de puertos 
// que contiene el buffer, seguido de los numeros de los puertos . Cada numero (inclutendo el primero son de 5 chars)
char* create_gossip_buffer(gossip_t** gossip_table){
    int gossip_table_size = get_gossip_table_size(gossip_table);
    int number_size = 5;
    int buffer_size = number_size * (gossip_table_size + 1) + 1; // +1 para el \0
    gossip_t* temp = *gossip_table;
    char* buffer = (char*)malloc(buffer_size);
    char* string_number = malloc(6);
    tostring(string_number, gossip_table_size);
    strcpy(buffer, string_number);
    for(int i = 1; i < gossip_table_size+1; i++){
        tostring(string_number, temp->number);
        strcat(buffer, string_number);
        temp = temp->next;
    }
    free(string_number);
    return buffer;
}

gossip_t* compare_gossip_tables(gossip_t** gossip_table1, gossip_t** gossip_table2){
    gossip_t* temp1 = *gossip_table1;
    gossip_t* temp2;

    // busco cada nodo de gp1 en gp2, si no esta, lo saco de gp1 porque quiere decir que se desconecto el nodo
    while(temp1 != NULL){
        temp2 = find_node(gossip_table2, temp1->number);
        if(temp2 == NULL){
            remove_node(gossip_table1, temp1);
        }
        temp1 = temp1->next; 
    }

    temp2 = *gossip_table2;
    while(temp2 != NULL){
        temp1 = find_node(gossip_table1, temp2->number);
        if(temp1 == NULL){
            gossip_t* new_node = create_node(temp2->number);
            // add_node(new_node, temp1);
        }
        temp2 = temp2->next;
    }
    free(gossip_table2); // ya no la voy a usar mas hasta que haga la proxima conexion y cree uno nuevo
}

void print_gossip_table(gossip_t* gossip_table){
	gossip_t* temp = gossip_table;
	printf("Gossip table: ");
	while(temp != NULL){
		printf("%d ", temp->number);
		temp = temp->next;		
	}
	printf("\n\n");
}

void* gossip(char* seed_port){
    // es mejor tener un array de char porque el ultimo elemento va a ser NULL y no me como un segfault cuando itero
    char** seed_ports = config_get_array_value(config, "PUERTO_SEEDS");
    memcpy(seed_ports[0], seed_port, 4); // ---- TODO: BORRAR
    char** seed_ips = config_get_array_value(config, "IP_SEEDS");
    int retardo_gossiping = config_get_int_value(config, "RETARDO_GOSSIPING") / 1000;
    while(1){
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
                remove_node(&GOSSIP_TABLE, find_node(&GOSSIP_TABLE, seed_port));
            }

            else{
                // TODO: comparar tablas y actualizar
                char* response = malloc(100);
                // mandarle 
                write(seed_socket, "GOSSIP", strlen("GOSSIP"));
                read(seed_socket, response, 80);
                log_info(logger, "Se logro conectar con el siguiente nodo");
                printf("%s", response);
                free(response);
            }
        }
        print_gossip_table(GOSSIP_TABLE);
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