#include <stdio.h>
#include <stdlib.h>
//#include <string.h>

int main(int argv, char** argc){
	if(argv < 5){
		printf("thread_runner <número_de_threads> <tamanho_do_buffer_global_em_kilobytes> <politica> <prioridade>\n");
		return 0;
	}
	int threads = atoi(argc[1]);
	int buffer = atoi(argc[2]);
	char* policy = argc[3];
	int prior = atoi(argc[4]);
	printf("%d threads | %dk buffer | %s priority %d", threads, buffer, policy, prior);
	return 0;
}