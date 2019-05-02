#include "pharser.h"

/*lee consola constantemente,recibe como argumento
un char* que contiene el nombre del prompt*/
void *console_input(void* name){
	while(1){

		char* buffer;
		size_t buffer_size = 3000;
		buffer = (char*)malloc(buffer_size * sizeof(char));
		printf("%s>", (char*) name);
		getline(&buffer, &buffer_size, stdin); //llamada bloqueante
		char* response = parse_bytearray(buffer); 
		printf("%s", response);
		free(buffer);

	} //se queda en escuhca constantenmente
	
}