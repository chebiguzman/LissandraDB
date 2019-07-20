#include "engine.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>
#include <commons/bitarray.h>

#define BLOCK_SIZE_DEFAULT 128
#define BLOCKS_AMOUNT_DEFAULT 12
char* MNT_POINT;
t_log* logg;
t_list* tables_name;
char* tables_path;
DIR* root;
char* bitmap_path;
int block_amount;
int block_size;

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
    bitmap_path = malloc(strlen(metadata_dir_path)+strlen("bitmap")+1);
    strcpy(bitmap_path,metadata_dir_path);
    strcat(bitmap_path,"bitmap");
   
    //consigo el directorio metadata
    char* meta_path = malloc(strlen(metadata_dir_path)+strlen("Metadata.bin")+1);
    strcpy(meta_path ,metadata_dir_path);
    strcat(meta_path, "Metadata.bin");

    free(metadata_dir_path);
 
    FILE* bitmap = fopen(bitmap_path,"r");
    if(bitmap==NULL){

        FILE* bitmap = fopen(bitmap_path,"w");
        char* bitearray;
        bitearray = string_repeat( '1', BLOCKS_AMOUNT_DEFAULT);
        fputs(bitearray, bitmap);
        fclose(bitmap);

    }else{

    }

    //free(bitmap_path);

    FILE* meta = fopen(meta_path, "r");
    //creo el archivo Metadata/Metadata.bin
    if(meta == NULL){
        meta = fopen(meta_path, "w");
        log_info(logg, "Se crea: %s", meta_path);
        char* text = "BLOCKS=%s\nBLOCK_SIZE=%s\nMAGIC_NUMBER=LISSANDRA\n";
        char* a = string_itoa(BLOCKS_AMOUNT_DEFAULT);
        char* b = string_itoa(BLOCK_SIZE_DEFAULT);
        char* r = malloc( strlen(text) + strlen(a) + strlen(b)+1);

        sprintf(r, text, a,b);
 
        fputs(r, meta);
 
        fclose(meta);

        free(r);
 
    }

    
    t_config* meta_config = config_create(meta_path);
    block_amount = config_get_int_value(meta_config, "BLOCKS");
    block_size = config_get_int_value(meta_config, "BLOCK_SIZE");

    free(meta_path);

    for(int i = 0; i < block_amount;i++){
        char* p = malloc(strlen(blocks_path)+strlen(string_itoa(block_amount))+5);
        strcpy(p, blocks_path);
        strcat(p, string_itoa(i));
        strcat(p, ".bin");
        check_or_create_file(p);
        free(p);
    }

    //free(blocks_path);
 
    DIR* tables_dir = opendir(tables_path);
    tables_name = list_create();

    //free(tables_path);

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
            //free(name);
         }
     }
 
   
}

int does_table_exist(char* table_name){
    char* q = strdup(table_name);
    string_to_upper(q);

    printf("DOES TABLE EXIST: %s", q);

    bool findTableByName(void* t){
        char* cmp = strdup((char*) t);
        string_to_upper(cmp);
        if(!strcmp(q, cmp)){
            free(cmp);
            return true;
        } 
        free(cmp); 
        return false;
    }

    char* t = list_find(tables_name,findTableByName);
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
    free(constante);

    log_info(logg,partitions);
    char* constante2= strdup(string_itoa(compactation_time));
    int util2=strlen(constante2)+strlen("\nCOMPACTATION");
    char* compactation= malloc(util2+3);
    strcpy(compactation,"\n");
    strcpy(compactation,"\nCOMPACTATION=");
    strcat(compactation,constante2);
    free(constante2);

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
    free(constante3);

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
        free(auxx);
        c--;
    }
    
    free(resp);

    list_add(tables_name, table_name);
    char* copy = strdup(table_name);
    string_to_upper(copy);

    fclose(metadata); 
    free(path_dir);
    free(partitions);
    free(consistencia);
    free(compactation);
    free(meta_path);

}

void engine_drop_table(char* table_name){
    char* q = strdup(table_name);
    string_to_upper(q);

    bool findTableByName(void* t){
        char* cmp = strdup((char*) t);
        string_to_upper(cmp);
        if(!strcmp(q, cmp)){
            free(cmp);
            return true;
        } 
        free(cmp); 
        return false;
    }

    list_remove_by_condition(tables_name,findTableByName);
    char* path = malloc(strlen(tables_path) + strlen(table_name) + 5);
    strcpy(path, tables_path);
    strcat(path, table_name);
    recursive_delete(path);

    free(q);
    free(path);
    return;

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
    free(table_path);
    return meta;
}

char* get_all_tables_metadata_as_string(){
    if(list_is_empty(tables_name)) return "";
    int tables_amount = list_size(tables_name);

    char* result = strdup("");

    for (size_t i = 0; i < tables_amount; i++)
    {   

        result = realloc(result, strlen(result) + strlen(list_get(tables_name,i))+ 8);
        strcat(result, "NOMBRE=");
        strcat(result, list_get(tables_name,i));

        result = realloc(result, strlen(result) + 2);
        strcat(result, "\n");
    
        char* m = get_table_metadata_as_string(list_get(tables_name,i));
        result = realloc(result, strlen(result) + strlen(m) + 1);
        strcat(result, m);
        result = realloc(result, strlen(result) + 4);
        strcat(result, ";\n\n");
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

    free(partition_name);

    t_table_partiton* parition = malloc(sizeof(t_table_partiton));
    t_config* c = config_create(partition_path);
    printf("assasd %s\n", partition_path);

    parition->blocks_size = config_get_long_value(c, "SIZE");

    parition->blocks = config_get_array_value(c, "BLOCKS");

    free(partition_path);

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
    free(meta_path);
    meta->consistency = config_get_int_value(c, "CONSISTENCY");
    meta->partition_number = config_get_int_value(c, "PARTICIONES");
    meta->compactation_time = config_get_long_value(c, "COMPACTATION");

    log_error(logg,"la consistencia %d", meta->consistency);
    log_error(logg, "numero de particiones %d", meta->partition_number);
    log_error(logg, "comapctation %dl",meta->compactation_time);


    return meta;
}

char* get_blocksize_table_rows(char* table_data){
    char* buffer = malloc(block_size);
    char** rows = string_split(table_data, "\n");
    strcpy(buffer, "");

    int buffer_size = 0;
    while(*rows){

        if((buffer_size+strlen(*rows)+1)>=block_size && buffer_size!=0){
            break;
        }

        strcat(buffer, *rows);
        strcat(buffer, "\n");

        buffer_size += strlen(*rows) + 1;

        rows++;

    
    }
    strcpy(table_data, "");


    while (*rows)
    {
        strcat(table_data,*rows);
        strcat(table_data, "\n");
        
        rows++;
    }
    return buffer;

}

void engine_dump_table(char* table_name, char* table_dump){ //esta funcion tiene que tomar el dump de cada table y llevarla al fs
    
    if(!does_table_exist(table_name)){ //chequeo que la tabla existe
        log_error(logg,"La tabla no existe");
        exit(-1);
    }
    char* blocks = strdup("");
    int dump_size = strlen(table_dump);
    while(table_dump[0] != '\0'){
        printf("el table dump antes del dp: %s\n", table_dump);
        char* r = get_blocksize_table_rows(table_dump);
        printf("table dump luego:%s\n", table_dump);
         //agrego los datos a uno o mas bloques -> ver bitmap
        int block = find_free_block(); //elijo un bloque libre
        char* block_name = string_itoa(block);
        char* block_path = malloc(sizeof(block_name)+sizeof(MNT_POINT)+sizeof("Bloques/")+sizeof(".bin"));
        
        block_path[0] = ('\0');
        strcat(block_path ,MNT_POINT);
        strcat(block_path ,"Bloques/");
        strcat(block_path ,block_name);
        strcat(block_path ,".bin");

        //escribo el dump en el bloque
        FILE* block_file = fopen(block_path,"r+");
        free(block_path);
        printf("respuesta: %s\n", r);

        fwrite(r,strlen(r),1,block_file);

        fclose(block_file);

        set_block_as_occupied(block);//marco el bloque como ocupado en el bitmap

        printf("se marco el bloqie como modificado\n");
        printf("blocks[0] es: %i",blocks[0]);
        
        if(blocks[0] == '\0'){
            //blocks = strdup(string_itoa(block));
            printf("blocks estaba vacia\n");
            strcat(blocks,string_itoa(block));
            printf("concatene blocks y block: %s\n", blocks);
        }else{
            char* blocks_buffer = malloc(strlen(blocks) + strlen(string_itoa(block))+1);
            strcpy(blocks_buffer, blocks );
            strcat(blocks_buffer, ",");
            strcat(blocks_buffer, string_itoa(block));
            free(blocks);
            blocks = blocks_buffer;
        }
        
        printf("termine de cargar los bloques\n");

        dump_size++; //por cada iteracion se agrega un \0

    }
    
   
    //creo los archivos tmp -> ver como nombro los archivos
    //chequeo si existen archivos con el nombre 0.tmp , 1.tmp, 2.tmp, etc... hasta encontrar uno que no exista
    char* tmp_path = malloc(sizeof(MNT_POINT)+sizeof("Tables/")+sizeof(table_name)+sizeof("/.tmp")+2);
   
    tmp_path[0] = '\0';
    strcat(tmp_path ,MNT_POINT);
    strcat(tmp_path ,"Tables/");
    strcat(tmp_path ,table_name);
    strcat(tmp_path ,"/");


    printf("El path temporal es: %s\n", tmp_path);

    //encuentro un archivo tmp: desde 0 en adelante itero hasta que no existe salgo y devuelvo el num
    char* tmp_file_number = string_itoa(find_tmp_name(tmp_path));

    char* tmp_filepath = malloc( strlen(tmp_path) + strlen(tmp_file_number) + strlen(".tmp") + 1);
    strcpy(tmp_filepath, tmp_path);
    strcat(tmp_filepath ,tmp_file_number);
    strcat(tmp_filepath ,".tmp");

    free(tmp_path);

    printf("tmp_filepath es: %s",tmp_filepath);

    FILE* tmp_file = fopen(tmp_filepath,"w+");//creo el archivo .tmp

    //cargo el archivo .tmp

    char* text = "SIZE=%s\nBLOCKS=[%s]\n";
    char* a = string_itoa(dump_size); //TODO ver que es size
    char* r = malloc( strlen(text) + strlen(a) + strlen(blocks)+1);

    sprintf(r, text, a,blocks);

    printf("va por aca: %s", r);
    
    fputs(r, tmp_file);

    fclose(tmp_file);
    free(tmp_filepath);
    free(r);
    free(table_dump);
    free(blocks);
    
    return;
}

//encuentro un archivo tmp: desde 0 en adelante itero hasta que no existe salgo y devuelvo el num
int find_tmp_name(char* tmp_path) { 
    int found = 0;
    int i = 0;
    while(1) {
        //genero el nombre
        char* tmp_name = string_itoa(i);
        char* tmp_filepath = malloc( strlen(tmp_path) + strlen(tmp_name)+ strlen(".tmp")+1);
        strcpy(tmp_filepath, tmp_path);
        //strcat(tmp_filepath, tmp_path);
        strcat(tmp_filepath ,tmp_name);
        strcat(tmp_filepath ,".tmp");

        if(!does_file_exist(tmp_filepath)){
            printf("encontre un nombre para tmp");
            free(tmp_filepath);
            return i;
        }

        i++;
    }
}

int find_free_block() {

    printf("El bloque %s\n", bitmap_path);
    FILE* bitmap_file = fopen(bitmap_path,"r+");
    char* bitmap = malloc(block_amount+2);
    

    fread(bitmap ,sizeof(char) ,block_amount ,bitmap_file);
    fclose(bitmap_file);
    for(int i = 0; i<block_amount ;i++) {
        if(bitmap[i]=='1'){ //recorro todo el bitmap buscando un 0
            free(bitmap);
            return i; //devuelvo el indice del primer bloque libre que encuentro -> es el numero de bloque libre
        }
    }    
    free(bitmap);
    log_error(logg,"No hay bloques libres");
    exit(-1); //ver bien que hacer cuando no hay bloques libres
}

void set_block_as_occupied(int block_number) {
          
    FILE* bitmap_file = fopen(bitmap_path,"r+");
    char *bitmap = malloc(block_amount+2);
    fread(bitmap, sizeof(char),block_amount, bitmap_file);
    fseek(bitmap_file, 0, SEEK_SET);
    bitmap[block_number] = '0';
    fwrite(bitmap, sizeof(char), block_amount, bitmap_file);
    fclose(bitmap_file);
    free(bitmap);
    return;
}

int does_file_exist(char* file_path){
    
    FILE* file = fopen(file_path, "r");
    
    if(file == NULL){
        return 0;
    }

    fclose(file);
    return 1;
}
