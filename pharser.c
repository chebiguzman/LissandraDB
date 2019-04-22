

#include "pharser.h"
#include "server.h"
#include "actions.h"
#include <string.h>
#define HEADER_BYTE_SIZE 9



typedef struct pakage_select pakage_select;

void pharse_bytearray(char* buffer){
    char header[HEADER_BYTE_SIZE];
    memcpy(header, buffer, HEADER_BYTE_SIZE);
    printf("header: %s\n", header );

    if(!strcmp(header,"SELECT")){
        pakage_select* pakage = malloc(sizeof(pakage_select));
        strcpy(pakage->header, header);
        
        int table_name_len = strlen(get_string_from_buffer(buffer, HEADER_BYTE_SIZE))+1; //sumo el \o
        pakage->table_name = strdup(get_string_from_buffer(buffer, HEADER_BYTE_SIZE));
        pakage->key = buffer[HEADER_BYTE_SIZE+table_name_len];


        printf("\n Datos de paquete:\n Header: %s\n Table name: %s\n Key: %d\n", pakage->header, pakage->table_name, buffer[HEADER_BYTE_SIZE+table_name_len]);
        action_select(pakage);
    }

    /*
    if(!strcmp(header,"INSERT")){
        
    }

    if(!strcmp(header,"CREATE")){
        
    }

    if(!strcmp(header,"DESCRIBE")){
        
    }

    if(!strcmp(header,"DROP")){
        
    }

    if(!strcmp(header,"JOURNAL")){
            
    }

    if(!strcmp(header,"ADD")){
            
    }

    if(!strcmp(header,"RUN")){
            
    }

    if(!strcmp(header,"METRICS")){
            
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