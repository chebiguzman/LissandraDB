

#include "pharser.h"
#include "server.h"
#include "actions.h"
#include <string.h>
#define INSTRUCTION_BYTE_SIZE 9



typedef struct package_select package_select;

void pharse_bytearray(char* buffer){
    char instruction[INSTRUCTION_BYTE_SIZE];
    memcpy(instruction, buffer, INSTRUCTION_BYTE_SIZE);
    printf("instruction: %s\n", instruction );

    if(!strcmp(instruction,"SELECT")){
        package_select* package = malloc(sizeof(package_select));
        strcpy(package->instruction, instruction);
        
        //TABLE_NAME
        int table_name_len = strlen(get_string_from_buffer(buffer, INSTRUCTION_BYTE_SIZE))+1; //sumo el \o
        package->table_name = strdup(get_string_from_buffer(buffer, INSTRUCTION_BYTE_SIZE));

        //KEY
        package->key = buffer[INSTRUCTION_BYTE_SIZE+table_name_len];


        printf("\n Datos de paquete:\n instruction: %s\n Table name: %s\n Key: %d\n", package->instruction, package->table_name, buffer[INSTRUCTION_BYTE_SIZE+table_name_len]);
        action_select(package);
    }

    
    if(!strcmp(instruction,"INSERT")){
        package_insert* package = malloc(sizeof(package_insert));
        strcpy(package->instruction, instruction);

        //TABLE_NAME
        int table_name_len = strlen(get_string_from_buffer(buffer, INSTRUCTION_BYTE_SIZE))+1; //sumo el \o
        package->table_name = strdup(get_string_from_buffer(buffer, INSTRUCTION_BYTE_SIZE));

        int tot_len = INSTRUCTION_BYTE_SIZE + table_name_len;

        //KEY
        package->key = buffer[tot_len];
        tot_len += sizeof(package->key);

        //VALUE
        int value_len = strlen(get_string_from_buffer(buffer, tot_len))+1;
        package->value = strdup(get_string_from_buffer(buffer, tot_len));

        action_insert(package);
    }

    if(!strcmp(instruction,"CREATE")){
        package_create* package = malloc(sizeof(package_create));
        strcpy(package->instruction, instruction);

        //TABLE_NAME
        int table_name_len = strlen(get_string_from_buffer(buffer, INSTRUCTION_BYTE_SIZE))+1; //sumo el \o
        package->table_name = strdup(get_string_from_buffer(buffer, INSTRUCTION_BYTE_SIZE));

        int tot_len = INSTRUCTION_BYTE_SIZE + table_name_len;

        //CONSISTENCY
        tot_len =+ strlen(get_string_from_buffer(buffer, tot_len))+1;
        package->consistency =(consistency_type) strdup(get_string_from_buffer(buffer, tot_len));

        package->partition_number = buffer[tot_len];
        tot_len += sizeof(package->partition_number);

        package->compactation_time = buffer[tot_len];

        action_create(package);
    }

    if(!strcmp(instruction,"DESCRIBE")){
        package_describe* package = malloc(sizeof(package_describe));
        strcpy(package->instruction, instruction);

        //TABLE_NAME
        int table_name_len = strlen(get_string_from_buffer(buffer, INSTRUCTION_BYTE_SIZE))+1; //sumo el \o
        package->table_name = strdup(get_string_from_buffer(buffer, INSTRUCTION_BYTE_SIZE));

        action_describe(package);
    }

    if(!strcmp(instruction,"DROP")){
        package_drop* package = malloc(sizeof(package_drop));
        strcpy(package->instruction, instruction);

        //TABLE_NAME
        int table_name_len = strlen(get_string_from_buffer(buffer, INSTRUCTION_BYTE_SIZE))+1; //sumo el \o
        package->table_name = strdup(get_string_from_buffer(buffer, INSTRUCTION_BYTE_SIZE));

        action_drop(package);
    }

    if(!strcmp(instruction,"JOURNAL")){
        package_journal* package = malloc(sizeof(package_journal));
        strcpy(package->instruction, instruction);

        action_journal(package);
    }

    if(!strcmp(instruction,"ADD")){
        package_add* package = malloc(sizeof(package_add));
        strcpy(package->instruction, instruction);

        //comparar que verdaderamente diga ADD MEMORY y TO y no otra cosa
        //Ya que usar inadecuadamente el comando no deberia estar permitido
        //Aca tenemos que santificar lo que nos entre, despues de este punto vamos a confiar 
        //en toda la informacion 
        action_add(package);
    }

    if(!strcmp(instruction,"RUN")){
        package_run* package = malloc(sizeof(package_run));
        strcpy(package->instruction, instruction);

        action_run(package);
    }

    if(!strcmp(instruction,"METRICS")){
        package_metrics* package = malloc(sizeof(package_metrics));
        strcpy(package->instruction, instruction);

        action_metrics(package);
    }
    
    
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
