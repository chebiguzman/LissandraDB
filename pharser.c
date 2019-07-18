
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

    //printf("\ninstruccion:%s\n",instruction); //La dejo para debug

    char buff[strlen(instr_buff)];
    memcpy(buff, instr_buff, strlen(instr_buff)+1);

    int i = 0;
    int offset = 0;
    bool last_character_was_null = false;
    bool openText = false;
    while(buff[i+offset] !='\0'){
        if(!openText){
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
                if(buff[i]=='"') openText = true;
            }
        }else{
            if(buff[i] == '"') openText = false;
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

    if(!strcmp(instruction,"SELECT")){
        if(parameters_length != 3) return "Numero de parametros incorrectos\n";

        package_select* package = malloc(sizeof(package_select));
        package->instruction = parameters[0];

        //TABLE_NAME
        package->table_name = parameters[1];

        //KEY
        package->key = atoi(parameters[2]);

        //printf("\n Datos de paquete:\n instruction: %s\n Table name: %s\n Key: %d\n", package->instruction, package->table_name,package->key);
        char* responce = action_select(package);
        return responce;
    }

    if(!strcmp(instruction,"RUN")){
        if(parameters_length != 2) return "Numero de parametros incorrectos\n";

        package_run* package = malloc(sizeof(package_run));
        package->instruction = parameters[0];

        //PATH
        package->path = parameters[1];
        printf("\n Datos de paquete:\n instruction: %s\n path: %s\n \n", package->instruction, package->path);
        char* responce = action_run(package);
        return responce;
    }

    if(!strcmp(instruction,"INSERT")){
        if(parameters_length != 4 && parameters_length != 5) return "Numero de parametros incorrectos\n";

        package_insert* package = malloc(sizeof(package_insert));
        package->instruction = parameters[0];
        
        //TABLE_NAME
        package->table_name = parameters[1];        
        //KEY
        package->key = atoi(parameters[2]);

        //VALUE
        package->value = parameters[3];

        char* value = malloc(strlen(parameters[3]));
        if(parameters[3][0] == '"'){
            int value_length = strlen(parameters[3]);
            if(parameters[3][value_length-1]=='"'){
                memcpy(value, parameters[3]+1, value_length-2);
                package->value = value;
            }else{
                free(value);
                return "Parametro value malformado.\n";
            }
        }
        
    
        //TIMESTAMP
        //si hay 4 parmateros me fijo si es un timestamp
        if(parameters_length == 5){
            unsigned long timestamp = atoi(parameters[4]);
            if(timestamp>0) package->timestamp = timestamp;
        }else{
            package->timestamp = time(NULL);
        }

       // printf("\n Datos de paquete:\n instruction: %s\n Table name: %s\n Key: %d\n Value: %s\n Timestamp: %lu\n", package->instruction, package->table_name, package->key, package->value, package->timestamp);
        char* responce = action_insert(package);
        return responce;
    }

    //CREATE [TABLA] [TIPO_CONSISTENCIA] [NUMERO_PARTICIONES] [COMPACTION_TIME]

    if(!strcmp(instruction,"CREATE")){
        if(parameters_length != 5 ) return "Numero de parametros incorrectos\n";

        package_create* package = malloc(sizeof(package_insert));
        package->instruction = parameters[0];
        
        //TABLE_NAME
        package->table_name = parameters[1];        
        //CONSISTENCIA
        char* c =  strdup(parameters[2]); 
        string_to_upper(c);
        if(!strcmp(c,"SC")){
            package->consistency = S_CONSISTENCY;
        }else if(!strcmp(c,"EC")){
            package->consistency = ANY_CONSISTENCY;
        }else if(!strcmp(c,"HC")){
            package->consistency = H_CONSISTENCY;
        }else{
            return "Criterio no valido";
        }

        //VALUE
        package->partition_number = atoi(parameters[3]);
        
        package->compactation_time = atol(parameters[4]);

        //printf("\n Datos de paquete:\n instruction: %s\n Table name: %s\n cons: %d\n particiones: %d\n comp: %lu\n", package->instruction, package->table_name, package->consistency, package->partition_number, package->compactation_time);
        
        char* responce =action_create(package);
 
        return responce;
    }

    if(!strcmp(instruction,"DESCRIBE")){
        package_describe* package = malloc(sizeof(package_describe));
        package->instruction = malloc(instruction_size);
        strcpy(package->instruction, instruction);

        //TABLE_NAME
        if(strlen(instr_buff) == strlen(instruction)+1) {
            package->table_name = NULL;
        } else {
            package->table_name = strdup(get_string_from_buffer(instr_buff, instruction_size));
        }

        printf("\n Datos de paquete:\n instruction: %s\n table name: %s\n \n", package->instruction, package->table_name);
        char* responce = action_describe(package);
        return responce;
    }

    if(!strcmp(instruction,"ADD")){
        if(parameters_length != 5) return "Numero de parametros incorrecto";

        package_add* package = malloc(sizeof(package_add));
        package->instruction = parameters[0];

        string_to_upper(parameters[1]);
        string_to_upper(parameters[3]);
        if(strcmp(parameters[1], "MEMORY")) return "Instrucion ADD mal formulada.\n";
        if(strcmp(parameters[3], "TO")) return "Instruccion ADD mal formulada.\n";
        
        package->id = atoi(strdup(parameters[2]));

        char* c =  strdup(parameters[4]); //+2 por TO
        string_to_upper(c);
        if(!strcmp(c,"SC")){
            package->consistency = S_CONSISTENCY;
        }else if(!strcmp(c,"EC")){
            package->consistency = ANY_CONSISTENCY;
        }else if(!strcmp(c,"HC")){
            package->consistency = H_CONSISTENCY;
        }else{
            return "Criterio no valido\n";
        }
        //printf("\n Datos de paquete:\n instruction: %s\n id: %d\ncriterio:%d \n", package->instruction , package->id, package->consistency);
        char* responce = action_add(package);
        return responce;
    }

    if(!strcmp(instruction,"MEMORY")){
        char* r =  action_intern__status();
        return r;
    }

    if(!strcmp(instruction, "DROP")){
        if(parameters_length != 2) return "Numero de parametros incorrecto";

        package_drop* package = malloc(sizeof(package_drop));
        package->table_name = parameters[1];


        return action_drop(package);
    }

    if(!strcmp(instruction, "JOURNAL")){
        if(parameters_length != 1) return "Numero de parametros incorrecto";

        package_journal* pakage = malloc(sizeof(package_journal));
        pakage->instruction = strdup("JOURNAL");
        return action_journal(pakage);
    }

    if(!strcmp(instruction, "METRICS")){
        if(parameters_length != 1) return "Numero de parametros incorrecto";
        package_metrics* pk = malloc(sizeof(package_metrics));
        pk->instruction = strdup("METRICS");
        

        return action_metrics(pk);
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
    memcpy(buffer, instr, strlen(instr));
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
    //char key[16]; //malisima idea llega a ser mas de 15 digitos y rompe
    char* key = string_itoa(package->key);
    char* val = strdup(package->value);
    char* timestmp = string_itoa(package->timestamp);
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

char* parse_package_describe(package_describe* pk){

    if(pk->table_name == NULL){
        char* buffer = malloc(strlen(pk->instruction) +4);
    
        strcat(buffer, pk->instruction);
        return buffer;
    }
    char* buffer = malloc(strlen(pk->instruction) + strlen(pk->table_name) +4);
    
    strcat(buffer, pk->instruction);
    strcat(buffer,sep);
    strcat(buffer,pk->table_name);
    return buffer;
}
#define BUFSIZE (sizeof(long) * 8 + 1)

char *ltoa(long N, char *str, int base)
{
      register int i = 2;
      long uarg;
      char *tail, *head = str, buf[BUFSIZE];

      if (36 < base || 2 > base)
            base = 10;                    /* can only use 0-9, A-Z        */
      tail = &buf[BUFSIZE - 1];           /* last character position      */
      *tail-- = '\0';

      if (10 == base && N < 0L)
      {
            *head++ = '-';
            uarg    = -N;
      }
      else  uarg = N;

      if (uarg)
      {
            for (i = 1; uarg; ++i)
            {
                  register ldiv_t r;

                  r       = ldiv(uarg, base);
                  *tail-- = (char)(r.rem + ((9L < r.rem) ?
                                  ('A' - 10L) : '0'));
                  uarg    = r.quot;
            }
      }
      else  *tail-- = '0';

      memcpy(head, ++tail, i);
      return str;
}

char* parse_package_create(package_create* pk){
    
 
    char* buffer = malloc( strlen(pk->instruction) + strlen(pk->table_name) + 3 /*consistencia */ + strlen(string_itoa(pk->partition_number) + 22 + 9));
     
    strcpy(buffer, pk->instruction);
    strcat(buffer, sep);
    strcat(buffer, pk->table_name);
    strcat(buffer, sep);
    char* consistency = malloc(4);
    switch (pk->consistency)
    {
    case S_CONSISTENCY:
        consistency = strdup("SC");
        break;
    case H_CONSISTENCY:
        consistency = strdup("HC");
    case ANY_CONSISTENCY:
        consistency = strdup("EC");
    
    default:
        consistency = strdup("ERR");
        break;
    }
    strcat(buffer, consistency );
    free(consistency);
    strcat(buffer, sep);
    strcat(buffer, string_itoa(pk->partition_number));
    strcat(buffer, sep);

    char* buff = malloc(30);
    ltoa(pk->compactation_time, buff, 10);

    strcat(buffer, buff );
    free(buff);
    return buffer;
}

char* parse_package_drop(package_drop* pk){
    char* bff = malloc( strlen(pk->instruction) + strlen(pk->table_name) + 4);
    strcpy(bff, pk->instruction);
    strcat(bff, sep);
    strcat(bff, pk->table_name);
    return bff;
}
char* parse_package_journal(package_journal* pk){
    return strdup(pk->instruction);
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

