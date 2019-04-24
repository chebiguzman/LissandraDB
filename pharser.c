
#include <stdio.h>
#include <stdlib.h>
#include "pharser.h"
#include "server.h"
#include "actions.h"
#include <string.h>



typedef struct package_select package_select;

char* pharse_bytearray(char* buffer){

    int i = 0;

    while (buffer[i] != '\0')
    {
        if(buffer[i]== '\n') buffer[i]='\0'; //quito las nuevas lineas 
        if(buffer[i]==' ') buffer[i] = '\0'; //quito los espacios

        i++;
    }
    
    int instruction_size = strlen( get_string_from_buffer(buffer,0))+1;
    char* instruction = malloc(instruction_size);

    strcpy(instruction, buffer);
    printf("instruction: %s\n",instruction );
    
    if(!strcmp(instruction,"SELECT")){
        package_select* package = malloc(sizeof(package_select));
        package->instruction = malloc(instruction_size);
        strcpy(package->instruction, instruction);
        
        //TABLE_NAME

        int table_name_len = strlen(get_string_from_buffer(buffer, instruction_size))+1; //sumo el \o
        package->table_name = strdup(get_string_from_buffer(buffer, instruction_size));
        
        //KEY
        
        char* key_tmp = strdup(get_string_from_buffer(buffer, instruction_size+table_name_len));
        package->key = atoi(key_tmp);
        free(key_tmp);
        
    

        printf("\n Datos de paquete:\n instruction: %s\n Table name: %s\n Key: %d\n", package->instruction, package->table_name,package->key);
        char* responce = action_select(package);
        return responce;
    }

    
    /*if(!strcmp(instruction,"INSERT")){
        package_insert* package = malloc(sizeof(package_insert));
        strcpy(package->instruction, instruction);

        //TABLE_NAME
        int table_name_len = strlen(get_string_from_buffer(buffer, instruction_size))+1; //sumo el \o
        package->table_name = strdup(get_string_from_buffer(buffer, instruction_size));

        int tot_len = instruction_size + table_name_len;

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
        int table_name_len = strlen(get_string_from_buffer(buffer, instruction_size))+1; //sumo el \o
        package->table_name = strdup(get_string_from_buffer(buffer, instruction_size));

        int tot_len = instruction_size + table_name_len;

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
        int table_name_len = strlen(get_string_from_buffer(buffer, instruction_size))+1; //sumo el \o
        package->table_name = strdup(get_string_from_buffer(buffer, instruction_size));

        action_describe(package);
    }

    if(!strcmp(instruction,"DROP")){
        package_drop* package = malloc(sizeof(package_drop));
        strcpy(package->instruction, instruction);

        //TABLE_NAME
        int table_name_len = strlen(get_string_from_buffer(buffer, instruction_size))+1; //sumo el \o
        package->table_name = strdup(get_string_from_buffer(buffer, instruction_size));

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
    }*/
    char* error_message = strdup("no es una instruccion valida\n");
    return error_message;
}



char* pharse_pakage_select(package_select* package){
    char* buffer; //separator for null terminations
    char sep[2] = {' '};
    char* instr = strdup(package->instruction);
    char* tbl_n = strdup(package->table_name);
    char key[16];
    sprintf(key, "%d", package->key);
    int tot_len = strlen(package->instruction)+1 + strlen(package->table_name)+1 + strlen(key)+1;
    
    // = strdup(itoa(package->key,10));
    buffer = malloc(tot_len);
    buffer = string_new();
    buffer = strcat(buffer, instr);
    buffer = strcat(buffer, sep); //emular NULL terminations
    buffer = strcat(buffer, tbl_n);
    buffer = strcat(buffer, sep);
    buffer = strcat(buffer, key);


}

char* create_buffer(int argc, char const *argv[]){
	char* buffer;
    int len = 0;
    if(argc-1 > 0){
        for (int i = 1; i < argc; i++)
        {
            len += strlen(argv[i])+1;
        }

        buffer = malloc(len);
        buffer = string_new();
        for (int i = 1; i < argc; i++)
        {
            strcat(buffer,argv[i]);
            strcat(buffer," ");
        }
        
        return buffer;
    }else{
        buffer = string_new();
        return buffer;
    }
    
}

//devuelve un string de un string de un array
char* get_string_from_buffer(char* buffer, int index){
    while (buffer[index] == '\0')
    {
        index++;
    }//mientras que la posicion inicial sea un fin de cadena moverse hacia adelante
    
	int endOfWord = index;
	while(buffer[endOfWord] != '\0' &&  buffer[endOfWord] != ' '){
		endOfWord ++;
	}
	char* bufferWord = malloc(endOfWord - index);
	memcpy(bufferWord, buffer + index, endOfWord);
	return bufferWord;
}
