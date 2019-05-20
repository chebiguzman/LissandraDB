#include <pthread.h>

typedef enum{
    S_CONSISTENCY,
    H_CONSISTENCY,
    ANY_CONSISTENCY
}t_consistency;

t_consistency getTableConsistency(char* table_name);
int get_loked_memory(t_consistency consistency);
void unlock_memory(int memoryfd);
