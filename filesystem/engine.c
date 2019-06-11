#include <stdlib.h>
#include <commons/log.h>
#include <pthread.h>
#include <string.h>
#include <commons/config.h>
#include "engine.h"
#include <sys/stat.h> //creacion de directorios
#include <sys/types.h> //creacion de directorios
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#define BLOCK_SIZE_DEFAULT 128
#define BLOCKS_AMOUNT_DEFAULT 12
char* MNT_POINT;
t_log* logg;
t_list* tables_upper_name;
t_list* tables_name;
char* tables_path;
DIR* root;

void check_or_create_dir(char* path){
    DIR* dir = opendir(path);
    if (dir != NULL) {
        closedir(dir);
    } else{
       int status = mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        log_info(logg, "se crea: %s", path);
        if(status!=0){
            log_error(logg, "Fatal error. No se puede escribir en el directorio. root?");
            exit(-1);
        }
 
    }
 
}
 
void engine_start(t_log* logger){

    logg = logger;
    t_config* config = config_create("config");
    MNT_POINT = config_get_string_value(config, "PUNTO_MONTAJE");
 
    DIR* mnt_dir = opendir(MNT_POINT);
 
    if(mnt_dir == NULL){
        log_error(logger, "Fatal error. El punto de montaje es invalido.");
        exit(-1);
    }else{
        closedir(mnt_dir);
       
    }
 
    //Armo los directorios
    char* metadata_dir_path = malloc(strlen(MNT_POINT)+strlen("Metadata/")+10);
    strcpy(metadata_dir_path, MNT_POINT);
    strcat(metadata_dir_path, "Metadata/");
          tables_path = malloc(strlen(MNT_POINT)+strlen("Tables/")+1);
    strcpy( tables_path,MNT_POINT);
    strcat(tables_path, "Tables/");
    char* blocks_path = malloc(strlen(MNT_POINT)+strlen("Bloques/")+1);
    strcpy(blocks_path , MNT_POINT);
    strcat(blocks_path, "Bloques/");
    
 
    check_or_create_dir(metadata_dir_path);
    check_or_create_dir(blocks_path);
    check_or_create_dir(tables_path);
 
 
    //Creo el archivo Metadata/Bitmap.bin
    char* bitmap_path = malloc(strlen(metadata_dir_path)+strlen("bitmap")+1);
    strcpy(bitmap_path,metadata_dir_path);
    strcat(bitmap_path,"bitmap");
   
    //consigo el directorio metadata
    char* meta_path = malloc(strlen(metadata_dir_path)+strlen("Metadata.bin")+1);
    strcpy(meta_path ,metadata_dir_path);
    strcat(meta_path, "Metadata.bin");
 
    FILE* bitmap = fopen(bitmap_path,"r");
    if(bitmap==NULL){

        FILE* bitmap = fopen(bitmap_path,"w");
        char* bitearray;
        bitearray = string_repeat( '0', BLOCKS_AMOUNT_DEFAULT);
        fputs(bitearray, bitmap);

        fclose(bitmap);

    }

    FILE* meta = fopen(meta_path, "r");
    //creo el archivo Metadata/Metadata.bin
    if(meta == NULL){
        meta = fopen(meta_path, "w");
        log_info(logg, "Se crea: %s", meta_path);
        char* text = "BLOCKS=%s\nBLOCK_SIZE=%s\nMAGIC_NUMBER=LISSANDRA\n";
        char* a = string_itoa(BLOCKS_AMOUNT_DEFAULT);
        char* b = string_itoa(BLOCK_SIZE_DEFAULT);
        char* r = malloc( strlen(text) + strlen(a) + strlen(b)+1);

        sprintf(r, "BLOCKS=%s\nBLOCK_SIZE=%s\nMAGIC_NUMBER=LISSANDRA\n", a,b);
        sprintf(r, text, b,a);
 
        fputs(r, meta);
 
        fclose(meta);
 
    }
 
    DIR* tables_dir = opendir(tables_path);
    tables_name = list_create();
    tables_upper_name = list_create();

    struct dirent *entry;
     while ((entry = readdir(tables_dir)) != NULL) {
         if(entry->d_type == DT_DIR){
             if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            char* name = malloc(strlen(entry->d_name) +1);
            strcpy(name, entry->d_name);
            list_add(tables_name, name);
            string_to_upper(entry->d_name);
            log_info(logg, entry->d_name);
            list_add(tables_upper_name,entry->d_name );
         }
     }
 
   
}

int does_table_exist(char* table_name){
    char* q = strdup(table_name);
    string_to_upper(q);

    bool findTableByName(void* t){
        if(!strcmp(q, (char*) t)) return true;
        return false;
    }

    char* t = list_find(tables_upper_name,findTableByName);
    free(q);
    if(t == NULL){
        return 0;
    }
    return 1;
}

int enginet_create_table(char* table_name, int consistency, int particiones, long compactation_time){
    
    //creo la carpeta
    int path_len = strlen(tables_path) + strlen(table_name) +1;
    char* path_dir =  malloc(path_len);

    strcpy(path_dir, tables_path);
    strcat(path_dir, table_name);
    
    log_error(logg, tables_path);
 
    check_or_create_dir(path_dir);
    
    //crea la carpeta ahora hay que rellenarla


    FILE * metadata;
    char* meta_path= malloc( strlen(path_dir) + strlen("metadata")+1);
    strcpy(meta_path, path_dir);
    strcat(meta_path,"/metadata");
    
    metadata=fopen(meta_path,"w+");

    if(metadata==NULL){
        log_error(logg, "Error al crear la tabla. Root?");
        exit(-1);
    }

    char* constante = strdup(string_itoa(particiones));

    int util=strlen(constante)+strlen("particiones");
    char* partitions=malloc(util +4);
    strcpy(partitions,"\n");
    strcpy(partitions,"PARTICIONES=");
    strcat(partitions,constante);
    log_info(logg,partitions);
    char* constante2= strdup(string_itoa(compactation_time));
    int util2=strlen(constante2)+strlen("\nCOMPACTATION");
    char* compactation= malloc(util2+3);
    strcpy(compactation,"\n");
    strcpy(compactation,"\nCOMPACTATION=");
    strcat(compactation,constante2);
    log_info(logg,compactation);
    char* constante3= strdup(string_itoa(consistency));
    char* consistencia;
    int larg0=strlen("sc=consistency\n");
    consistencia= malloc(larg0+5);

    switch(atoi(constante3)){
    case 0:
        strcpy(consistencia,"CONSISTENCY=SC\n");
        break;
    case 1:
        strcpy(consistencia,"CONSISTENCY=SH\n");
        break;
    case 2:
        strcpy(consistencia,"CONSISTENCY=ANY\n");
        break;
    }

    log_info(logg,consistencia);
    fputs(consistencia,metadata);
    fputs(partitions,metadata);
    fputs(compactation,metadata);
    int c= particiones;
    c--;

    char* resp=malloc(1000);
    char* metadata_template = "SIZE=0\nBLOCKS=[]\n";

    while(c>=0){
        char* auxx=NULL;
        auxx=strdup(string_itoa(c));
        strcpy(resp, path_dir);
        strcat(resp,"/");
        strcat(resp,auxx);
        strcat(resp,".part");
        log_info(logg,resp);
        FILE* part = fopen(resp,"w+");
        fputs(metadata_template, part);
        fclose(part);
        c--;
    }
 
    free(resp);

    list_add(tables_name, table_name);
    char* copy = strdup(table_name);
    string_to_upper(copy);
    list_add(tables_upper_name, copy);

    fclose(metadata); 
    free(path_dir);
    free(partitions);
    free(consistencia);
    free(meta_path);

}

char* get_table_metadata_as_string(char* table_name){

    char* table_path = malloc( strlen(table_name) + strlen(tables_path) + strlen("/metadata") +1);
    strcpy(table_path, tables_path);
    strcat(table_path, table_name);
    strcat(table_path, "/metadata");
    FILE* f = fopen(table_path, "r");
    fseek(f, 0L, SEEK_END);
    int bytes = ftell(f);

    fseek(f, 0l, SEEK_SET);
    char* meta = (char*)calloc(bytes, sizeof(char));	
    fread(meta, sizeof(char), bytes, f);

    fclose(f);
    return meta;
}

char* get_all_tables_metadata(){
    if(list_is_empty(tables_name)) return "";
    int tables_amount = list_size(tables_name);

    char* result = strdup("");

    for (size_t i = 0; i < tables_amount; i++)
    {   

        result = realloc(result, strlen(result) + strlen(list_get(tables_name,i)));

        strcat(result, list_get(tables_name,i));

        result = realloc(result, strlen(result) + 2);
        strcat(result, "\n");
    
        char* m = get_table_metadata_as_string(list_get(tables_name,i));
        result = realloc(result, strlen(result) + strlen(m) + 1);
        strcat(result, m);
        result = realloc(result, strlen(result) + 4);
        strcat(result, "\n\n");
        free(m); 
    }



    return result;
}