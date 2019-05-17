
#include <stdio.h>
#include <stdlib.h>
#include "pharser.h"
#include "server.h"
#include "actions.h"
#include <string.h>
#include <time.h>



char sep[2] = {' '};//separator for null terminations
char* exec_instr(char* instr_buff){

    int instruction_size = strlen( get_string_from_buffer(instr_buff,0))+1;
 
    char* instruction = malloc(instruction_size);
    strcpy(instruction, get_string_from_buffer(instr_buff,0));
    //printf("\ninstruccion:%s\n",instruction); //La dejo para debug


    if(!strcmp(instruction,"SELECT")){
        package_select* package = malloc(sizeof(package_select));
        package->instruction = malloc(instruction_size);
        strcpy(package->instruction, instruction);
        
        //TABLE_NAME

        int table_name_len = strlen(get_string_from_buffer(instr_buff, instruction_size))+1; //sumo el \o
        package->table_name = strdup(get_string_from_buffer(instr_buff, instruction_size));
        
        //KEY
        
        char* key_tmp = strdup(get_string_from_buffer(instr_buff, instruction_size+table_name_len));
        package->key = atoi(key_tmp);
        free(key_tmp);
        
    

        printf("\n Datos de paquete:\n instruction: %s\n Table name: %s\n Key: %d\n", package->instruction, package->table_name,package->key);
        char* responce = action_select(package);
        return responce;
    }

    if(!strcmp(instruction,"RUN")){
        package_run* package = malloc(sizeof(package_run));
        package->instruction = malloc(instruction_size);
        strcpy(package->instruction, instruction);

        //PATH
        package->path = strdup(get_string_from_buffer(instr_buff, instruction_size));
        printf("\n Datos de paquete:\n instruction: %s\n path: %s\n \n", package->instruction, package->path);
        char* responce = action_run(package);
        return responce;
    }

    if(!strcmp(instruction,"INSERT")){
        package_insert* package = malloc(sizeof(package_insert));
        package->instruction = malloc(instruction_size);
        strcpy(package->instruction, instruction);
        
        //TABLE_NAME
        int table_name_len = strlen(get_string_from_buffer(instr_buff, instruction_size))+1; //sumo el \o
        package->table_name = strdup(get_string_from_buffer(instr_buff, instruction_size));
        
        //KEY
        char* key_tmp = strdup(get_string_from_buffer(instr_buff, instruction_size+table_name_len));
        int key_len = strlen(key_tmp)+1;
        package->key = atoi(key_tmp);
        free(key_tmp);

        //VALUE
        int value_len = strlen(get_value_from_buffer(instr_buff, instruction_size+table_name_len+key_len))+2;
        package->value = strdup(get_value_from_buffer(instr_buff, instruction_size+table_name_len+key_len));
        
        //TIMESTAMP
        char* timestamp_tmp = strdup(get_string_from_buffer(instr_buff,instruction_size+table_name_len+key_len+value_len));
        if (timestamp_tmp != '\0') {
            package->timestamp = atoi(timestamp_tmp);
        } else {
            package->timestamp = (unsigned)time(NULL);
        }
        free(timestamp_tmp);
    

        printf("\n Datos de paquete:\n instruction: %s\n Table name: %s\n Key: %d\n Value: %s\n Timestamp: %u\n", package->instruction, package->table_name, package->key, package->value, package->timestamp);
        char* responce = action_insert(package);
        return responce;
    }

    char* error_message = strdup("no es una instruccion valida\n");
    return error_message;
}

char* parse_package_select(package_select* package){
    char* buffer; 
    char* instr = strdup(package->instruction);
    char* tbl_n = strdup(package->table_name);
    char key[16];
    sprintf(key, "%d", package->key);
    int tot_len = strlen(package->instruction)+1 + strlen(package->table_name)+1 + strlen(key)+1;
    
    buffer = malloc(tot_len);
    buffer = string_new();
    buffer = strcat(buffer, instr);
    buffer = strcat(buffer, sep); //emular NULL terminations
    buffer = strcat(buffer, tbl_n);
    buffer = strcat(buffer, sep);
    buffer = strcat(buffer, key);
    return buffer;
}

char* parse_package_run(package_run* pk){
    char* buffer;
    char* instr = strdup(pk->instruction);
    char* path = strdup(pk->path);
    int tot_len = strlen(instr) + strlen(instr) +2;
    buffer = malloc(tot_len);
    buffer[0] = '\0';
    strcat(buffer, instr);
    strcat(buffer,sep);
    strcat(buffer,path);
    ;;
}

char* parse_package_insert(package_insert* package){
    char* buffer; 
    char* instr = strdup(package->instruction);
    char* tbl_n = strdup(package->table_name);
    char key[16];
    sprintf(key, "%d", package->key);
    char* val = strdup(package->value);
    char timestmp[16];
    sprintf(timestmp, "%u", package->timestamp);
    int tot_len = strlen(package->instruction)+1 + strlen(package->table_name)+1 + strlen(key)+1 + strlen(package->value)+1 + strlen(timestmp)+1;
    
    buffer = malloc(tot_len);
    buffer = string_new();
    buffer = strcat(buffer, instr);
    buffer = strcat(buffer, sep); //emular NULL terminations
    buffer = strcat(buffer, tbl_n);
    buffer = strcat(buffer, sep);
    buffer = strcat(buffer, key);
    buffer = strcat(buffer, sep);
    buffer = strcat(buffer, val);
    buffer = strcat(buffer, sep);
    buffer = strcat(buffer, timestmp);
    return buffer;
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
//TODO en la copia del puntero se produce un  memeory leack
char* get_string_from_buffer(char* buffer, int index){

    char* bufferWord = string_substring_from(buffer,index);
    bufferWord = strdup(bufferWord);

    char buff_tmp[strlen(bufferWord)];
    memcpy(buff_tmp, bufferWord, strlen(bufferWord)+1);
    
    int i = 0;

    while (buff_tmp[i] != '\0'){

        if(buff_tmp[i]== '\n') buff_tmp[i]='\0'; //quito las nuevas lineas 
        if(buff_tmp[i]==' ') buff_tmp[i] = '\0'; //quito los espacios

        i++;
    }

    bufferWord = strdup(buff_tmp);

	return bufferWord;
}

char* get_value_from_buffer(char* buffer, int index){
    char* bufferWord = string_substring_from(buffer,index);
    bufferWord = strdup(bufferWord);

    char buff_tmp[strlen(bufferWord)];
    memcpy(buff_tmp, bufferWord, strlen(bufferWord)+1);
    
    int i = 0;

    while (buff_tmp[i] != '\0'){

        if(buff_tmp[i]== '\n') buff_tmp[i]='\0'; //quito las nuevas lineas 
        if(buff_tmp[i]==' ') buff_tmp[i] = '\0'; //quito los espacios

        i++;
    }

    bufferWord = strdup(buff_tmp);

	return bufferWord;
}