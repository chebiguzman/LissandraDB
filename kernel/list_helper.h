
typedef struct 
{
    int position;
    t_list* list;
}t_list_helper;

t_list_helper* list_helper_init(t_list* ls);
void* list_helper_next(t_list_helper* helper);
t_kmemory* list_helper_find_memory_by_fd(t_list_helper* helper, int fd);
bool has_memory_fd(int fd, t_kmemory* mem);