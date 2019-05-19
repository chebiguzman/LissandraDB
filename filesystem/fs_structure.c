#include <stdlib.h>
#include <commons/log.h>
#include <pthread.h>
#include <string.h>
#include <commons/config.h>
#include "fs_structure.h"
#include <sys/stat.h> //creacion de directorios
#include <sys/types.h> //creacion de directorios

#include <errno.h>
#include <unistd.h>

void* setup_fs(void* args) {

    fs_structure_info* fs_structure_info = args;

    //Punto de montaje.
    t_config* config = config_create("config");
    char* MNT_POINT = config_get_string_value(config, "PUNTO_MONTAJE");

    //crear directorios y archivos (si no existen)

    //METADATA
    char* metadata_dir_name = "Metadata/";
    char path_metadata[300];
    path_metadata[0] = '\0';
    strcat(path_metadata,MNT_POINT);
    strcat(path_metadata,metadata_dir_name);
    //log_info(fs_structure_info->logger, path_metadata);
    int crear_directorio_metadata = mkdir(path_metadata, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); //read/write/search permisions for owner and group and read/search for others
    if (crear_directorio_metadata != 0) {
        char* errorsito = strerror(errno);
        log_error(fs_structure_info->logger, errorsito );
    } else {
        log_info(fs_structure_info->logger, "El directorio Metadata se creo exitosamente");
    }

    //Creo archivo Metadata/Metadata.bin
    char* metadata_file_name = "Metadata.bin";
    char path_metadata_file[300];
    path_metadata_file[0] = '\0';
    strcat(path_metadata_file,MNT_POINT);
    strcat(path_metadata_file,metadata_dir_name);
    strcat(path_metadata_file,metadata_file_name);
    if (access(path_metadata_file, F_OK) == -1) { //si el archivo no existe
        //crear el archivo
        FILE* fp_m;
        fp_m = fopen(path_metadata_file,"w");
        fclose(fp_m);
        log_info(fs_structure_info->logger, "El archivo Metadata/Metadata.bin se creo exitosamente");
    }

    //Creo el archivo Metadata/Bitmap.bin
    char* bitmap_file_name = "Bitmap.bin";
    char path_bitmap_file[300];
    path_bitmap_file[0] = '\0';
    strcat(path_bitmap_file,MNT_POINT);
    strcat(path_bitmap_file,metadata_dir_name);
    strcat(path_bitmap_file,bitmap_file_name);
    if (access(path_bitmap_file, F_OK) == -1) { //si el archivo no existe
        //crear el archivo
        FILE* fp_b;
        fp_b = fopen(path_bitmap_file,"w");
        fclose(fp_b);
        log_info(fs_structure_info->logger, "El archivo Metadata/Bitmap.bin se creo exitosamente");
    }

    //TABLES
    char* tables_dir_name = "Tables/";
    char path_tables[300];
    path_tables[0] = '\0';
    strcat(path_tables,MNT_POINT);
    strcat(path_tables,tables_dir_name);
    //log_info(fs_structure_info->logger, path_tables);
    int crear_directorio_tables = mkdir(path_tables, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); //read/write/search permisions for owner and group and read/search for others
    if (crear_directorio_tables != 0) {
        char* errorsito = strerror(errno);
        log_error(fs_structure_info->logger, errorsito );
    } else {
        log_info(fs_structure_info->logger, "El directorio Tables se creo exitosamente");
    }

    //BLOQUES
    char* bloques_dir_name = "Bloques/";
    char path_bloques[300];
    path_bloques[0] = '\0';
    strcat(path_bloques,MNT_POINT);
    strcat(path_bloques,bloques_dir_name);
    //log_info(fs_structure_info->logger, path_bloques);
    int crear_directorio_bloques = mkdir(path_bloques, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); //read/write/search permisions for owner and group and read/search for others
    if (crear_directorio_bloques != 0) {
        char* errorsito = strerror(errno);
        log_error(fs_structure_info->logger, errorsito );
    } else {
        log_info(fs_structure_info->logger, "El directorio Bloques se creo exitosamente");
    }

    return 0;


    /*free(metadata_dir_name);
    free(tables_dir_name);
    free(bloques_dir_name);
    free(MNT_POINT);
    config_destroy(config);*/

}