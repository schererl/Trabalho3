#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/sched.h>

#define SIZE_BUF 10000
#define N_THREADS 5
volatile int running = 1;

char  mem[SIZE_BUF];
int index_mem = 0; //semaforo
pthread_mutex_t simLock, lock;

int gambiarra = 0;

void *run(void *data)
{
	//iniciar todas threads "simultaneamente"
	pthread_mutex_lock(&simLock);
	pthread_mutex_unlock(&simLock);
	
	char d = 'a'+(int)data;
	while (running){
		
		pthread_mutex_lock(&lock);
		if(index_mem >= SIZE_BUF){
			pthread_mutex_unlock(&lock);
			break;
		}
		mem[index_mem] = d;
		index_mem+=1;
		pthread_mutex_unlock(&lock);
	}
			
	return 0;
}

void printMem(char *mem){
	char last_char = mem[0];
	int char_counter[N_THREADS] = {0};
	
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

	for(int i = 0; i < N_THREADS; i++){
		printf("\t%c: %d\n", 'a' + i, char_counter[i]);
	}	
}


void print_sched(int policy)
{
	int priority_min, priority_max;

	switch(policy){
		case SCHED_DEADLINE:
			printf("SCHED_DEADLINE");
			break;
		case SCHED_FIFO:
			printf("SCHED_FIFO");
			break;
		case SCHED_RR:
			printf("SCHED_RR");
			break;
		case SCHED_NORMAL:
			printf("SCHED_OTHER");
			break;
		case SCHED_BATCH:
			printf("SCHED_BATCH");
			break;
		case SCHED_IDLE:
			printf("SCHED_IDLE");
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
		printf("Invalid priority: MIN: %d, MAX: %d", sched_get_priority_min(newpolicy), sched_get_priority_max(newpolicy));

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

int main(int argc, char **argv)
{
	int timesleep;
	pthread_t tid[N_THREADS];
	
	if (argc < 2){
		printf("usage: ./%s <execution_time>\n\n", argv[0]);

		return 0;
	}

	if (pthread_mutex_init(&lock, NULL) != 0 || pthread_mutex_init(&simLock, NULL) != 0) {
	        printf("\n mutex init has failed\n");
        	return 1;
	}

	int erro;

	
	//timesleep = atoi(argv[1]);
	for(int i = 0; i < N_THREADS; i++){
		//printf("create %d %c\n", i, 'a'+i);
		
		erro = pthread_create(&tid[i], NULL, run, (void*)i);
		
		if(erro != 0)
			printf("THREAD ERRO MOTHERFUCKER\n");

		//setpriority(&tid[i], SCHED_OTHER,0);
		
		
		if(i == 3){
		
			setpriority(&tid[i], SCHED_RR, sched_get_priority_max(SCHED_RR));
		}	
		else{
			setpriority(&tid[i], SCHED_OTHER, 0);
		}
		
		
		
	}

	//sleep(timesleep);
	//running = 0;

	for(int i = 0; i < N_THREADS; i++){
		pthread_join(tid[i], NULL);
		printf("done %d\n",i);
	}
	pthread_mutex_destroy(&lock);
	pthread_mutex_destroy(&simLock);

	//printf("%s\n", mem);
	
	/*
	for(int i = 0; i < SIZE_BUF; i++){
		printf("%c", mem[i]);
	}
	*/
	printMem(mem);
	printf("\n");
	return 0;
}
