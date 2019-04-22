

#include "pharser.h"
#include "server.h"
#include "actions.h"
#include <string.h>
#define HEADER_BYTE_SIZE 9



typedef struct package_select package_select;

void pharse_bytearray(char* buffer){
    char header[HEADER_BYTE_SIZE];
    memcpy(header, buffer, HEADER_BYTE_SIZE);
    printf("header: %s\n", header );

    if(!strcmp(header,"SELECT")){
        package_select* package = malloc(sizeof(package_select));
        strcpy(package->header, header);
        
        int table_name_len = strlen(get_string_from_buffer(buffer, HEADER_BYTE_SIZE))+1; //sumo el \o
        package->table_name = strdup(get_string_from_buffer(buffer, HEADER_BYTE_SIZE));
        package->key = buffer[HEADER_BYTE_SIZE+table_name_len];


        printf("\n Datos de paquete:\n Header: %s\n Table name: %s\n Key: %d\n", package->header, package->table_name, buffer[HEADER_BYTE_SIZE+table_name_len]);
        action_select(package);
    }

    /*
    if(!strcmp(header,"INSERT")){
        package_select* package = malloc(sizeof(package_insert));
        strcpy(package->header, header);

        action_insert(package);
    }

    if(!strcmp(header,"CREATE")){
        create_package* package = malloc(sizeof(package_create));
        strcpy(package->header, header);

        action_create(package);
    }

    if(!strcmp(header,"DESCRIBE")){
        package_describe* package = malloc(sizeof(package_describe));
        strcpy(package->header, header);

        action_describe(package);
    }

    if(!strcmp(header,"DROP")){
        package_drop* package = malloc(sizeof(package_drop));
        strcpy(package->header, header);

        action_drop(package);
    }

    if(!strcmp(header,"JOURNAL")){
        package_journal* package = malloc(sizeof(package_journal));
        strcpy(package->header, header);

        action_journal(package);
    }

    if(!strcmp(header,"ADD")){
        package_add* package = malloc(sizeof(package_add));
        strcpy(package->header, header);

        action_add(package);
    }

    if(!strcmp(header,"RUN")){
        package_run* package = malloc(sizeof(package_run));
        strcpy(package->header, header);

        action_run(package);
    }

    if(!strcmp(header,"METRICS")){
        package_metrics* package = malloc(sizeof(package_metrics));
        strcpy(package->header, header);

        action_metrics(package);
    }*/

    
}


char* create_buffer(int argc, char const *argv[]){
	int argSize = 0;
	for(int i = 0; i < argc; i++){
		argSize += sizeof(argv[i]);
	}
	char* buffer = malloc(argSize);
	memcpy(buffer, argv, argSize);
	return buffer;
}

//devuelve un string de un string de un array
char* get_string_from_buffer(char* buffer, int index){
	int endOfWord = index;
	while(buffer[endOfWord] != '\0'){
		endOfWord ++;
	}
	char* bufferWord = malloc(endOfWord - index-2);
	memcpy(bufferWord, buffer + index, endOfWord);
	return bufferWord;
}
