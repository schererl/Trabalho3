#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/sched.h>

int SIZE_BUF;// 1000000
char*  mem;
int index_mem = 0; 
pthread_mutex_t simLock, lock;


void *run(void *data)
{
	//iniciar todas threads "simultaneamente"
	pthread_mutex_lock(&simLock);
	pthread_mutex_unlock(&simLock);
	
	char d = 'a'+(int)data;
	while (index_mem <= SIZE_BUF){		
		pthread_mutex_lock(&lock);
		mem[index_mem] = d;
		index_mem+=1;
		pthread_mutex_unlock(&lock);
	}
	pthread_mutex_unlock(&lock);	
	return 0;
}

void printMem(char *mem, const int threads){
	char last_char = mem[0];
	//int char_counter[threads] = {0};
	int *char_counter = malloc(sizeof(int) * threads);

	printf("\n%c", mem[0]);
	
	char_counter[(int)last_char - (int)'a'] += 1;
	for(int i = 1; i < SIZE_BUF;i++){
		if(last_char != mem[i]){
			last_char = mem[i];
			printf("%c", mem[i]);
		}
				
		char_counter[(int)last_char - (int)'a'] += 1;
	}

	printf("\n\nCHAR COUNTER:\n");

	for(int i = 0; i < threads; i++){
		printf("\t%c: %d\n", 'a' + i, char_counter[i]);
	}	
}


void print_sched(int policy)
{
	int priority_min, priority_max;

	switch(policy){
		case SCHED_NORMAL:
			printf("SCHED_OTHER");
			break;
		case SCHED_FIFO:
			printf("SCHED_FIFO");
			break;
		case SCHED_RR:
			printf("SCHED_RR");
			break;
		case SCHED_BATCH:
			printf("SCHED_BATCH");
			break;
		case SCHED_IDLE:
			printf("SCHED_IDLE");
			break;
		case SCHED_DEADLINE:
			printf("SCHED_DEADLINE");
			break;
		default:
			printf("unknown\n");
	}
	priority_min = sched_get_priority_min(policy);
	priority_max = sched_get_priority_max(policy);
	printf(" PRI_MIN: %d PRI_MAX: %d\n", priority_min, priority_max);
}

int setpriority(pthread_t *thr, int newpolicy, int newpriority)
{
	int policy, ret;
	struct sched_param param;

	if (newpriority > sched_get_priority_max(newpolicy) || newpriority < sched_get_priority_min(newpolicy)){
		printf("Invalid priority: MIN: %d, MAX: %d ==> %d (%d)\n", sched_get_priority_min(newpolicy), sched_get_priority_max(newpolicy), newpriority, newpolicy);

		return -1;
	}


	pthread_getschedparam(*thr, &policy, &param);
	printf("current: ");
	print_sched(policy);

	param.sched_priority = newpriority;
	ret = pthread_setschedparam(*thr, newpolicy, &param);
	if (ret != 0)
		perror("perror(): ");

	pthread_getschedparam(*thr, &policy, &param);
	printf("new: ");
	print_sched(policy);
	printf("priority: %d\n", newpriority);

	return 0;
}

int get_policy(char* policy){
	int def = -1;
	switch(policy[6]){
		case 'o':
		case 'O':
			return SCHED_OTHER;
		case 'N':
		case 'n':
			return SCHED_NORMAL;
		case 'f':
		case 'F':
			return SCHED_FIFO;
		case 'R':
		case 'r':
			return SCHED_RR;
		case 'i':
		case 'I':
			return SCHED_IDLE;
		case 'b':
		case 'B':
			return SCHED_BATCH;
		case 'd':
		case 'D':
			return SCHED_DEADLINE;
		default:
			return def;
	}	
}

int main(int argc, char **argv)
{
	if(argc < 5){

		printf("thread_runner <número_de_threads> <tamanho_do_buffer_global_em_kilobytes> <politica1> <prioridade1> ... <politicaN> <prioridadeN>\n");

		return 0;
	}

	const int threads = atoi(argv[1]);
	//int SIZE_BUF = atoi(argv[2]);
	SIZE_BUF = atoi(argv[2]);
	mem = (char*)malloc(sizeof(char)*SIZE_BUF);

	if(argc < (threads*2)+3){

		printf("número de argumentos insuficientes:\n");
		printf("thread_runner <número_de_threads> <tamanho_do_buffer_global_em_kilobytes> <politica1> <prioridade1> ... <politicaN> <prioridadeN>\n");		

		return 0;
	}

	char* policy[threads];
	int prior[threads];

	for(int t = 0; t < threads; t++){

		int offset = (2*t);
		policy[t] = get_policy(argv[3+offset]);
		prior[t]  = atoi(argv[4+offset]);

	}		
	
	pthread_t tid[threads];

	if (pthread_mutex_init(&lock, NULL) != 0 || pthread_mutex_init(&simLock, NULL) != 0) {
	        printf("\n mutex init has failed\n");
        	return 1;
	}
	pthread_mutex_lock(&simLock);

	int erro;
	for(int i = 0; i < threads; i++){
		//printf("create %d %c\n", i, 'a'+i);
		
		erro = pthread_create(&tid[i], NULL, run, (void*)i);
		
		if(erro != 0){
			printf("THREAD ERRO\n");
			return 1;
		}
		setpriority(&tid[i], policy[i], prior[i]);
		
	}

	pthread_mutex_unlock(&simLock);
	for(int i = 0; i < threads; i++){
		pthread_join(tid[i], NULL);
		//if(i+1 == N_THREADS)
		//printf("done %d\n",i);
	}
	pthread_mutex_destroy(&lock);
	pthread_mutex_destroy(&simLock);
	printMem(mem, threads);
	free(mem);
	printf("\n");
	return 0;
}
