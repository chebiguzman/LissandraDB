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
	setbuf(stdout, NULL);

	while(1){
		//printf("a punto de bloquear\n");

		
		char* buffer;
		size_t buffer_size = 3000;
		buffer = (char*)malloc(buffer_size * sizeof(char));
		//pthread_mutex_lock(&self->lock);
		/*if(self->cont_int==0){
			//printf("consola entra en espera\n");
			pthread_cond_wait(&self->cond, &self->lock);
			pthread_mutex_unlock(&self->lock);
		}else{
			self->cont_int = 0;
			pthread_mutex_unlock(&self->lock);
		}
		*/

		//pthread_mutex_lock(&self->print_lock);

		pthread_mutex_lock(&self->lock);
		
		if(self->cont_int==0){
			pthread_cond_wait(&self->cond, &self->lock);
			self->cont_int = 0;
			printf("\r%s>", self->name);
			pthread_mutex_unlock(&self->lock);
		}else{
			self->cont_int = 0;
			usleep(10000);
			printf("\r%s>", self->name);
			//fprintf(stderr,"\r%s>", self->name);
			pthread_mutex_unlock(&self->lock);
		}

		
		//pthread_mutex_unlock(&self->print_lock);

		getline(&buffer, &buffer_size, stdin); //llamada bloqueante
		parse_input(buffer); //los resultados se imprimen en exec.
		
		free(buffer);
		//pthread_cond_wait(&self->cond, &self->lock);


	} //se queda en escuhca constantenmente
	
}