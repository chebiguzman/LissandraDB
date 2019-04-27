void *read_console(void *buffer){
	size_t buffer_size = 3000;
	buffer = (char*)malloc(buffer_size * sizeof(char));
	getline(&buffer, &buffer_size, stdin);
}