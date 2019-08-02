#include "metrics_worker.h"
#include "kmemory.h"
#include <commons/string.h>
#include <commons/log.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include "../pharser.h"
t_metrics* m;
t_log* logger;
void metrics_start(t_log* log){
    logger = log;
    m = malloc(sizeof(t_metrics));
    m->memory_count_ec = 0;
    m->memory_count_hc = 0;
    m->memory_count_sc = 0;

    m->read_count_ec = 0;
    m->read_count_sc = 0;
    m->read_count_hc = 0;

    m->total_count_ec = 0;
    m->total_count_hc = 0;
    m->total_count_sc = 0;

    m->write_count_ec = 0;
    m->write_count_sc = 0;
    m->write_count_hc = 0;

    m->memory_count_ec = dictionary_create();
    m->memory_count_hc = dictionary_create();
    m->memory_count_sc = dictionary_create();

    m->read_latency_ec = list_create();
    m->read_latency_sc = list_create();
    m->read_latency_hc = list_create();

    m->write_latency_ec = list_create();
    m->write_latency_sc = list_create();
    m->write_latency_hc = list_create();
}

void register_select(int memoryid, t_consistency c, double* latency){
    switch (c)
    {
    case S_CONSISTENCY:
        m->read_count_sc++;
        m->total_count_sc++;
        list_add(m->read_latency_sc, latency);
        if(dictionary_has_key(m->memory_count_sc, string_itoa(memoryid))){
            int* i = dictionary_get(m->memory_count_sc, string_itoa(memoryid));
            int r = *i;
            r++;
            *i = r;

        }else{
            int* i = malloc(sizeof(int));
            *i = 1;
            dictionary_put(m->memory_count_sc, string_itoa(memoryid), i);
        }
        break;

    case H_CONSISTENCY:
        m->read_count_hc++;
        m->total_count_hc++;
        list_add(m->read_latency_hc, latency);
        if(dictionary_has_key(m->memory_count_hc, string_itoa(memoryid))){
            int* i = dictionary_get(m->memory_count_hc, string_itoa(memoryid));
            int r = *i;
            r++;
            *i = r;

        }else{
            int* i = malloc(sizeof(int));
            *i = 1;
            dictionary_put(m->memory_count_hc, string_itoa(memoryid), i);
        }
        break;

    case ANY_CONSISTENCY:
        m->read_count_ec++;
        m->total_count_ec++;
        list_add(m->read_latency_ec, latency);
        if(dictionary_has_key(m->memory_count_ec, string_itoa(memoryid))){
            int* i = dictionary_get(m->memory_count_ec, string_itoa(memoryid));
            int r = *i;
            r++;
            *i = r;

        }else{
            int* i = malloc(sizeof(int));
            *i = 1;
            dictionary_put(m->memory_count_ec, string_itoa(memoryid), i);
        }
        break;
    }
}

    void register_insert(int memoryid, t_consistency c, double* latency){
     switch (c)
    {
    case S_CONSISTENCY:
        m->write_count_sc++;
        m->total_count_sc++;
        list_add(m->write_latency_sc, latency);
        if(dictionary_has_key(m->memory_count_sc, string_itoa(memoryid))){
            int* i = dictionary_get(m->memory_count_sc, string_itoa(memoryid));
            int r = *i;
            r++;
            *i = r;

        }else{
            int* i = malloc(sizeof(int));
            *i = 1;
            dictionary_put(m->memory_count_sc, string_itoa(memoryid), i);
        }
        break;

    case H_CONSISTENCY:
        m->write_count_hc++;
        m->total_count_hc++;
        list_add(m->write_latency_hc, latency);
        if(dictionary_has_key(m->memory_count_hc, string_itoa(memoryid))){
            int* i = dictionary_get(m->memory_count_hc, string_itoa(memoryid));
            int r = *i;
            r++;
            *i = r;

        }else{
            int* i = malloc(sizeof(int));
            *i = 1;
            dictionary_put(m->memory_count_hc, string_itoa(memoryid), i);
        }
        break;

    case ANY_CONSISTENCY:
        m->write_count_ec++;
        m->total_count_ec++;
        list_add(m->write_latency_ec, latency);
        if(dictionary_has_key(m->memory_count_ec, string_itoa(memoryid))){
            int* i = dictionary_get(m->memory_count_ec, string_itoa(memoryid));
            int r = *i;
            r++;
            *i = r;

        }else{
            int* i = malloc(sizeof(int));
            *i = 1;
            dictionary_put(m->memory_count_ec, string_itoa(memoryid), i);
        }
        break;
    }
}

void* print_metrics(void* args){

}

double list_promedy(t_list* list){
    double sum = 0;

    if(list_size(list)>0){
        for(int i = 0; i < list_size(list);i++){

            sum += *((double *)list_get(list, i));
        }

        return sum/list_size(list);
    }
    return sum;
   
}
#define BUFSIZE (sizeof(double) * 8 + 1)



char* get_metrics(){
    char* r = malloc(10000);
    strcpy(r, "\n-----------------------------\nStrong consistency:\nRead latency:");

    double rlsc = list_promedy(m->read_latency_sc );

    char* buff = malloc(30);
    sprintf(buff, "%f",rlsc);
    strcat(r, buff);
    free(buff);
    //log_error(logger, r);


    strcat(r, "ms\nWrite latency:");

    double wlsc = list_promedy(m->write_latency_sc );
    buff = malloc(300);
    sprintf(buff, "%f",wlsc);
    strcat(r, buff);
    free(buff);

    strcat(r, "ms\nReads:");
    strcat(r, string_itoa(m->read_count_sc));
    strcat(r, "\nWrites:");
    strcat(r, string_itoa(m->write_count_sc));
    strcat(r, "\nMemory loads:\n");

    void memory_concatenator_sc(char* id ,void* count){
        strcat(r, id);
        strcat(r,":");
        int c = *((int *) count);
        strcat(r, string_itoa((c*100)/m->total_count_sc));
        strcat(r, "%%\n");
    }

    dictionary_iterator(m->memory_count_sc,memory_concatenator_sc );

    strcat(r, "\nHash strong consistency:\nRead latency:");

    double rlhc = list_promedy(m->read_latency_hc );

    buff = malloc(30);
    sprintf(buff, "%f",rlhc);
    strcat(r, buff);
    free(buff);
    //log_error(logger, r);


    strcat(r, "ms\nWrite latency:");

    double wlhc = list_promedy(m->write_latency_hc );
    buff = malloc(300);
    sprintf(buff, "%f",wlhc);
    strcat(r, buff);
    free(buff);

    strcat(r, "ms\nReads:");
    strcat(r, string_itoa(m->read_count_hc));
    strcat(r, "\nWrites:");
    strcat(r, string_itoa(m->write_count_hc));
    strcat(r, "\nMemory loads:\n");

    void memory_concatenator_hc(char* id ,void* count){
        strcat(r, id);
        strcat(r,":");
        int c = *((int *) count);
        strcat(r, string_itoa((c*100)/m->total_count_hc));
        strcat(r, "%%\n");
    }

    dictionary_iterator(m->memory_count_hc,memory_concatenator_hc );

    strcat(r, "\nEventual consistency:\nRead latency:");

    double rlec = list_promedy(m->read_latency_ec );

    buff = malloc(30);
    sprintf(buff, "%f",rlec);
    strcat(r, buff);
    free(buff);
    //log_error(logger, r);


    strcat(r, "ms\nWrite latency:");

    double wlec = list_promedy(m->write_latency_ec );
    buff = malloc(300);
    sprintf(buff, "%f",wlec);
    strcat(r, buff);
    free(buff);

    strcat(r, "ms\nReads:");
    strcat(r, string_itoa(m->read_count_ec));
    strcat(r, "\nWrites:");
    strcat(r, string_itoa(m->write_count_ec));
    strcat(r, "\nMemory loads:\n");

    void memory_concatenator_ec(char* id ,void* count){
        strcat(r, id);
        strcat(r,":");
        int c = *((int *) count);
        strcat(r, string_itoa((c*100)/m->total_count_ec));
        strcat(r, "%%\n");
    }

    dictionary_iterator(m->memory_count_ec,memory_concatenator_ec );

    strcat(r, "-------------------------------------------\n");
    return r;
}
