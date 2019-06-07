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
t_list* tables_name;
char* root_dirr;
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
    char* tables_path = malloc(strlen(MNT_POINT)+strlen("Tables/")+1);
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
 
    /*DIR* tables_dir = opendir(tables_path);
    tables_name = list_create();
    struct dirent *entry;
     while ((entry = readdir(tables_dir)) != NULL) {
         if(entry->d_type == DT_DIR){
             if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
                string_to_upper(entry->d_name);
            list_add(tables_name,entry->d_name );
         }
     }*/
 
   
}
 
 
//CREATE [root_dirrTABLA] [TIPO_CONSISTENCIA] [NUMERO_PARTICIONES] [COMPACTION_TIME]
int enginet_create_table(char* table_name, char* consistency, int particiones, long compactation_time){
    
    char* path_dir = strdup(root_dirr);
    strcat(path_dir, table_name);
    char* meta_path = strdup(path_dir);
   
    printf("%s\n", path_dir);
 
    check_or_create_dir(path_dir);
    //crea la carpeta ahora hay que rellenarla
}