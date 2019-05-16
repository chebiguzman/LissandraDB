#ifndef PHARSER_H_   
#define PHARSER_H_
#include "server.h"

char* action_select(package_select* select_info);

char* action_insert(package_insert* insert_info);

void action_create(package_create* create_info);

void action_describe(package_describe* describe_info);

void action_drop(package_drop* drop_info);

void action_journal(package_journal* journal_info);

void action_add(package_add* add_info);

char* action_run(package_run* run_info);

void action_metrics(package_metrics* metrics_info);


/* Funciones armadas para copiar
void action_select(package_select* select_info){
  log_info(logger, "Se recibio una accion select");
}

void action_insert(package_insert* insert_info){
  log_info(logger, "Se recibio una accion insert");
}

void action_create(package_create* create_info){
  log_info(logger, "Se recibio una accion create");
}

void action_describe(package_describe* describe_info){
  log_info(logger, "Se recibio una accion describe");
}

void action_drop(package_drop* drop_info){
  log_info(logger, "Se recibio una accion drop");
}

void action_journal(package_journal* journal_info){
  log_info(logger, "Se recibio una accion select");
}

void action_add(package_add* add_info){
  log_info(logger, "Se recibio una accion select");
}

void action_run(package_run* run_info){
  log_info(logger, "Se recibio una accion run");
}

void action_metrics(package_metrics* metrics_info){
  log_info(logger, "Se recibio una accion metrics");
}
*/


#endif
