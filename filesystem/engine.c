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

char* MNT_POINT;
t_log* logg;
t_list* tables_name;
char* root_dirr;
DIR* root;
DIR* open_or_create_dir(char* path){
    DIR* dir = opendir(path);
    if (dir) {
        return dir;
    } else{
       int status = mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        log_info(logg, "se crea: %s", path);
        if(status!=0){
            log_error(logg, "Fatal error. No se puede escribir en el directorio. root?");
            exit(-1);
        }
       return open_or_create_dir(path);
    }

}

void engine_start(t_log* logger){
     //Punto de montaje.
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
    char* metadata_dir = strdup(MNT_POINT);
    strcat(metadata_dir, "Metadata/");
    char* tables_dir = strdup(MNT_POINT);
    strcat(tables_dir, "Tables/");
    char* blocks_dir = strdup(MNT_POINT);
    strcat(blocks_dir, "Bloques/");

    root_dirr = tables_dir;

    //directorios abiertos
    open_or_create_dir(metadata_dir);
     root = open_or_create_dir(tables_dir);
    open_or_create_dir(blocks_dir);


    //Creo el archivo Metadata/Bitmap.bin
    char* bitmap_file = strdup("");
    strcat(bitmap_file,metadata_dir);
    strcat(bitmap_file,"bitmap");
    printf("%s\n",bitmap_file);

    //consigo el directorio metadata
    char* meta_file = strdup("");
    strcat(meta_file, metadata_dir);
    strcat(meta_file, "Metadata.bin");
    printf("%s\n",meta_file);


    FILE* bitmap = fopen(bitmap_file,"w");
    FILE* meta = fopen(meta_file, "r");

    //creo el archivo Metadata/Metadata.bin
    if(meta == NULL){
        meta = fopen(meta_file, "w");
        log_info(logg, "Se crea: %s", meta_file);

        char* a = strdup("60");
        char* b = strdup("5145");
        char* r = malloc(3000);

        sprintf(r, "BLOCKS=%s\nBLOCK_SIZE=%s\nMAGIC_NUMBER=LISSANDRA\n", b,a);
        fputs(r, meta);

        fclose(meta);

    }

    tables_name = list_create();
    struct dirent *entry;
     while ((entry = readdir(root)) != NULL) {
         if(entry->d_type == DT_DIR){
             if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
                string_to_upper(entry->d_name);
            list_add(tables_name,entry->d_name );
         }
     }

    
}

int generate_new_table(char* table_name, char* consistency, int particiones, long compactation_time){
    char* path_dir = strdup(root_dirr);
    strcat(path_dir, table_name);
    char* meta_path = strdup(path_dir);
    
    printf("%s\n", path_dir);

    open_or_create_dir(path_dir);
    //crea la carpeta ahora hay que rellenarla

}

//CREATE [root_dirrTABLA] [TIPO_CONSISTENCIA] [NUMERO_PARTICIONES] [COMPACTION_TIME]
int engineroot_dirr_create_table(char* table_name, char* consistency, int particiones, long compactation_time){
    bool find_table_by_name(void* table){
        string_to_upper(table_name);
        if(!strcmp(table, table_name)){
           return true;
        }
        return false;
    }

    char* table = list_find(tables_name, find_table_by_name);
    if(table == NULL){
        return generate_new_table(table_name, consistency,  particiones,compactation_time);
    }else{
        return -1;
    }
}

