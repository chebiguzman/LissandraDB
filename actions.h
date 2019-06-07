#ifndef PHARSER_H_   
#define PHARSER_H_
#include "server.h"

char* parse_input(char* input);

char* action_select(package_select* select_info);

char* action_insert(package_insert* insert_info);

char* action_create(package_create* create_info);

char* action_describe(package_describe* describe_info);

char* action_drop(package_drop* drop_info);

char* action_journal(package_journal* journal_info);

char* action_add(package_add* add_info);

char* action_run(package_run* run_info);

char* action_metrics(package_metrics* metrics_info);

char* action_intern_memory_status();

#endif
