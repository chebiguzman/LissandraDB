#include "pharser.h"
#include "console.h"
/*lee consola constantemente,recibe como argumento
un char* que contiene el nombre del prompt*/
void *console_input(void* name){
	while(1){

		char* buffer;
		size_t buffer_size = 3000;
		buffer = (char*)malloc(buffer_size * sizeof(char));
		printf("\r%s>", (char*) name);
		getline(&buffer, &buffer_size, stdin); //llamada bloqueante
		char* response = parse_input(buffer); 
		printf("\r%s", response);
		free(response);
		free(buffer);

	} //se queda en escuhca constantenmente
	
}

void *console_input_wait(void* args){
	t_console_control* self = args;
	while(1){
		
		char* buffer;
		size_t buffer_size = 3000;
		buffer = (char*)malloc(buffer_size * sizeof(char));
		printf("\r%s>", self->name);
		getline(&buffer, &buffer_size, stdin); //llamada bloqueante
		parse_input(buffer); //los resultados se imprimen en exec.
		pthread_cond_wait(&self->cond, &self->lock);
		free(buffer);
		

	} //se queda en escuhca constantenmente
	
}