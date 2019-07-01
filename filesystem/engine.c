#include "engine.h"
 #include <sys/types.h>
       #include <sys/stat.h>
       #include <fts.h>
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

void check_or_create_file(char* path){
    FILE* file = fopen(path,"r");
    if(file==NULL){
        file = fopen(path,"w");
        log_info(logg, "se crea: %s", path);
    }

    fclose(file);
}

int recursive_delete(const char *dir){
    int ret = 0;
    FTS *ftsp = NULL;
    FTSENT *curr;

    // Cast needed (in C) because fts_open() takes a "char * const *", instead
    // of a "const char * const *", which is only allowed in C++. fts_open()
    // does not modify the argument.
    char *files[] = { (char *) dir, NULL };

    // FTS_NOCHDIR  - Avoid changing cwd, which could cause unexpected behavior
    //                in multithreaded programs
    // FTS_PHYSICAL - Don't follow symlinks. Prevents deletion of files outside
    //                of the specified directory
    // FTS_XDEV     - Don't cross filesystem boundaries
    ftsp = fts_open(files, FTS_NOCHDIR | FTS_PHYSICAL | FTS_XDEV, NULL);
    if (!ftsp) {
        fprintf(stderr, "%s: fts_open failed: %s\n", dir, strerror(errno));
        ret = -1;
        goto finish;
    }

    while ((curr = fts_read(ftsp))) {
        switch (curr->fts_info) {
        case FTS_NS:
        case FTS_DNR:
        case FTS_ERR:
            fprintf(stderr, "%s: fts_read error: %s\n",
                    curr->fts_accpath, strerror(curr->fts_errno));
            break;

        case FTS_DC:
        case FTS_DOT:
        case FTS_NSOK:
            // Not reached unless FTS_LOGICAL, FTS_SEEDOT, or FTS_NOSTAT were
            // passed to fts_open()
            break;

        case FTS_D:
            // Do nothing. Need depth-first search, so directories are deleted
            // in FTS_DP
            break;

        case FTS_DP:
        case FTS_F:
        case FTS_SL:
        case FTS_SLNONE:
        case FTS_DEFAULT:
            if (remove(curr->fts_accpath) < 0) {
                fprintf(stderr, "%s: Failed to remove: %s\n",
                        curr->fts_path, strerror(errno));
                ret = -1;
            }
            break;
        }
    }

    finish:
        if (ftsp) {
            fts_close(ftsp);
        }

        return ret;
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

    
    t_config* meta_config = config_create(meta_path);
    int block_amount = config_get_int_value(meta_config, "BLOCKS");

    for(int i = 0; i < block_amount;i++){
        char* p = malloc(strlen(blocks_path)+strlen(string_itoa(block_amount))+5);
        strcpy(p, blocks_path);
        strcat(p, string_itoa(i));
        strcat(p, ".bin");
        check_or_create_file(p);
        free(p);
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
        strcpy(consistencia,"CONSISTENCY=0\n");
        break;
    case 1:
        strcpy(consistencia,"CONSISTENCY=1\n");
        break;
    case 2:
        strcpy(consistencia,"CONSISTENCY=2\n");
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

void engine_drop_table(char* table_name){
    char* q = strdup(table_name);
    string_to_upper(q);
    bool findTableByName(void* t){
        if(!strcmp(q, (char*) t)) return true;
        return false;
    }

    list_remove_by_condition(tables_upper_name,findTableByName);
    char* path = malloc(strlen(tables_path) + strlen(table_name) + 5);
    strcpy(path, tables_path);
    strcat(path, table_name);
    recursive_delete(path);

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

char* get_all_tables_metadata_as_string(){
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

t_table_partiton* get_table_partition(char* table_name, int table_partition_number){
    char* partition_name = strdup(string_itoa(table_partition_number));
    char* partition_path =malloc(strlen(tables_path) + strlen(table_name) + 1 + strlen(partition_name)+ strlen(".part") + 5);
    strcpy(partition_path ,tables_path);

    strcat(partition_path,table_name);
    strcat(partition_path,"/");
    strcat(partition_path, partition_name);
    strcat(partition_path, ".part");

    t_table_partiton* parition = malloc(sizeof(t_table_partiton));
    t_config* c = config_create(partition_path);
    printf("assasd %s\n", partition_path);

    parition->blocks_size = config_get_long_value(c, "SIZE");

    parition->blocks = config_get_array_value(c, "BLOCKS");

    return parition;
}

t_table_metadata* get_table_metadata(char* table_name){
    t_table_metadata* meta = malloc(sizeof(t_table_metadata));
    char* meta_path = malloc(strlen(tables_path) + strlen(table_name) + strlen("/metadata") + 1);
    strcpy(meta_path, tables_path);
    strcat(meta_path, table_name);
    strcat(meta_path, "/metadata");

    log_info(logg, meta_path);
    
    t_config* c = config_create(meta_path);
    meta->consistency = config_get_int_value(c, "CONSISTENCY");
    meta->partition_number = config_get_int_value(c, "PARTICIONES");
    meta->compactation_time = config_get_long_value(c, "COMPACTATION");

    log_error(logg,"la consistencia %d", meta->consistency);
    log_error(logg, "numero de particiones %d", meta->partition_number);
    log_error(logg, "comapctation %dl",meta->compactation_time);


    return meta;
}