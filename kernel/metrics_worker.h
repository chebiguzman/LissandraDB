#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
typedef struct
{
    int read_count_sc;
    int read_count_hc;
    int read_count_ec;

    int write_count_sc;
    int write_count_hc;
    int write_count_ec;

    int total_count_sc;
    int total_count_hc;
    int total_count_ec;

    t_dictionary* memory_count_sc;
    t_dictionary* memory_count_hc;
    t_dictionary* memory_count_ec;

    t_list* read_latency_sc;
    t_list* read_latency_hc;
    t_list* read_latency_ec;

    t_list* write_latency_sc;
    t_list* write_latency_hc;
    t_list* write_latency_ec;

}t_metrics;

void metrics_start();
char* get_metrics();