
#include <stdio.h>
#include <stdlib.h>
#include "pharser.h"
#include "server.h"
#include "actions.h"
#include<string.h>




char sep[2] = {' '};//separator for null terminations
char* exec_instr(char* instr_buff){

    int instruction_size = strlen( get_string_from_buffer(instr_buff,0))+1;
 
    char* instruction = malloc(instruction_size);
    strcpy(instruction, get_string_from_buffer(instr_buff,0));
    string_to_upper(instruction);

    printf("\ninstruccion:%s\n",instruction); //La dejo para debug

    char buff[strlen(instr_buff)];
    memcpy(buff, instr_buff, strlen(instr_buff)+1);

    int i = 0;
    int offset = 0;
    bool last_character_was_null = false;
    while(buff[i+offset] !='\0'){
        
        if(buff[i]== '\n' || buff[i]==' '){

            buff[i]= '|';
            buff[i] = '|';

            if(last_character_was_null){
                int len = strlen(buff)+1;
                char* tmp = string_substring_from(buff,i+1);
                memcpy(&buff[i], tmp, strlen(tmp)+1);
                i--;
            }

            last_character_was_null = true;

        }else{
            buff[i] = buff[i];   
            last_character_was_null = false;
      
        }

        i++;        
        //quito las nuevas lineas 
        //quito los espacios 
    }
    buff[i-offset+1] = '\0';
    
    //printf("es \n%s\n", buff);

    
    char** parameters = string_split(buff, "|");
    int parameters_length = 0;
    while (parameters[parameters_length] != NULL){
        parameters_length++;
    }
    printf("%s\n", buff);
    printf("%d\n", parameters_length);
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

    if(!strcmp(instruction,"ADD")){
        package_add* package = malloc(sizeof(package_add));
        package->instruction = malloc(instruction_size);
        strcpy(package->instruction, instruction);

        if(parameters_length != 5) return "Numero de parametros incorrecto";
        string_to_upper(parameters[1]);
        string_to_upper(parameters[3]);
        if(strcmp(parameters[1], "MEMORY")) return "Instrucion ADD mal formulada";
        if(strcmp(parameters[3], "TO")) return "Instruccion ADD mal formulada";
        
        char* ids = strdup(parameters[2]);
        //printf("id:%s\n", ids);
        package->id = atoi(ids);
        free(ids);

        char* c =  strdup(parameters[4]); //+2 por TO
        string_to_upper(c);
        printf("c:%s\n", c);
        if(!strcmp(c,"SC")){
            package->consistency = S_CONSISTENCY;
        }else if(!strcmp(c,"ANY")){
            package->consistency = ANY_CONSISTENCY;
        }else if(!strcmp(c,"HC")){
            package->consistency = H_CONSISTENCY;
        }else{
            return "Criterio no valido";
        }
        //free(instr_buff);
        printf("\n Datos de paquete:\n instruction: %s\n id: %d\ncriterio:%d \n", package->instruction , package->id, package->consistency);
        char* responce = action_add(package);
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

