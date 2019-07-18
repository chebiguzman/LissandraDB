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

void register_select(int* memoryid, t_consistency c, long* latency){
    switch (c)
    {
    case S_CONSISTENCY:
        m->read_count_sc++;
        m->total_count_sc++;
        list_add(m->read_latency_sc, latency);
        if(dictionary_has_key(m->memory_count_sc, string_itoa(*memoryid))){
            int* i = dictionary_get(m->memory_count_sc, string_itoa(*memoryid));
            *i++;
    
        }else{
            int* i = malloc(sizeof(int));
            *i = 1;
            dictionary_put(m->memory_count_sc, string_itoa(*memoryid), i);
        }
        break;

   /* case H_CONSISTENCY:
        m->read_count_hc++;
        m->total_count_hc++;
        list_add(m->read_latency_hc, latency);
        if(dictionary_has_key(m->memory_count_hc, string_itoa(memoryid))){
            int* i = dictionary_get(m->memory_count_hc, string_itoa(memoryid));
            *i++;
    
        }else{
            int* i = malloc(sizeof(int));
            *i = 1;
            dictionary_put(m->memory_count_hc, memoryid, i);
        }
        break;
    
    case ANY_CONSISTENCY:
        m->read_count_ec++;
        m->total_count_ec++;
        list_add(m->read_latency_ec, latency);
        if(dictionary_has_key(m->memory_count_ec, string_itoa(memoryid))){
            int* i = dictionary_get(m->memory_count_ec, string_itoa(memoryid));
            *i++;
    
        }else{
            int* i = malloc(sizeof(int));
            *i = 1;
            dictionary_put(m->memory_count_ec, memoryid, i);
        }
        break;*/
        
    }

  /*  void register_insert(int memoryid, t_consistency c, long latency){
    switch (c)
    {
    case S_CONSISTENCY:
        m->write_count_sc++;
        m->total_count_sc++;
        list_add(m->write_latency_sc, latency);
        if(dictionary_has_key(m->memory_count_sc, string_itoa(memoryid))){
            int* i = dictionary_get(m->memory_count_sc, string_itoa(memoryid));
            *i++;
    
        }else{
            int* i = malloc(sizeof(int));
            *i = 1;
            dictionary_put(m->memory_count_sc, memoryid, i);
        }
        break;

    case H_CONSISTENCY:
        m->write_count_hc++;
        m->total_count_hc++;
        list_add(m->write_latency_hc, latency);
        if(dictionary_has_key(m->memory_count_hc, string_itoa(memoryid))){
            int* i = dictionary_get(m->memory_count_hc, string_itoa(memoryid));
            *i++;
    
        }else{
            int* i = malloc(sizeof(int));
            *i = 1;
            dictionary_put(m->memory_count_hc, memoryid, i);
        }
        break;
    
    case ANY_CONSISTENCY:
        m->write_count_ec++;
        m->total_count_ec++;
        list_add(m->write_latency_ec, latency);
        if(dictionary_has_key(m->memory_count_ec, string_itoa(memoryid))){
            int* i = dictionary_get(m->memory_count_ec, string_itoa(memoryid));
            *i++;
    
        }else{
            int* i = malloc(sizeof(int));
            *i = 1;
            dictionary_put(m->memory_count_ec, memoryid, i);
        }
        break;
        
    }*/
}

void* print_metrics(void* args){

}

long list_promedy(t_list* list){
    long sum = 0;

    if(list_size(list)>0){
        for(int i = 0; i < list_size(list);i++){

            sum += *((long *)list_get(list, i));
        }

        return sum/list_size(list);
    }
    return sum;
   
}
#define BUFSIZE (sizeof(long) * 8 + 1)



char* get_metrics(){
    char* r = malloc(3000);
    strcpy(r, "Strong consistency:\nRead latency:");

    long rlsc = list_promedy(m->read_latency_sc );

    char* buff = malloc(300);
    strcat(r, ltoa(rlsc, buff, 10));
    //log_error(logger, r);

    free(buff);
    strcat(r, "ms\nWrite latency:");

    long wlsc = list_promedy(m->write_latency_sc );
    buff = malloc(300);
    strcat(r, ltoa(wlsc, buff, 10));
    free(buff);

    strcat(r, "ms\nReads:");
    strcat(r, string_itoa(m->read_count_sc));
    strcat(r, "\nWrites:");
    strcat(r, string_itoa(m->write_count_sc));
    strcat(r, "\nMemory loads:\n");

    void memory_concatenator(char* id ,void* count){
        strcat(r, id);
        strcat(r,":");
        int c = *((int *) count);
        strcat(r, string_itoa((c*100)/m->total_count_sc));
        strcat(r, "%%\n");
    }

    dictionary_iterator(m->memory_count_sc,memory_concatenator );

    return r;
}
