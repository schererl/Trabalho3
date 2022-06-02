#include <stdio.h>
#include <stdlib.h>
//#include <string.h>

int main(int argv, char** argc){
	if(argv < 5){
		printf("thread_runner <número_de_threads> <tamanho_do_buffer_global_em_kilobytes> <politica1> <prioridade1> ... <politicaN> <prioridadeN>\n");
		return 0;
	}
	int threads = atoi(argc[1]);
	int buffer = atoi(argc[2]);
	if(argv < (threads*2)+3){
		printf("bagulho das thread insuficiente arrombado\n");		
		printf("thread_runner <número_de_threads> <tamanho_do_buffer_global_em_kilobytes> <politica1> <prioridade1> ... <politicaN> <prioridadeN>\n");		
		return 0;
	}
	
	char* policy[threads];
	int prior[threads];
	for(int t = 0; t < threads; t++){
		int offset = (2*t);
		policy[t] = argc[3+offset];
		prior[t] = atoi(argc[4+offset]);
	}
	
	printf("\t%d threads | %dk buffer\n", threads, buffer);
	for(int t = 0; t < threads; t++){
		printf("Thread %d [%s prior %d]\n", t, policy[t], prior[t]);
	}
	
	return 0;
}