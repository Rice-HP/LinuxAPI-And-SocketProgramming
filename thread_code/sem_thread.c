#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

int value[6] = { 0 };
int beginnum = 100;
sem_t s_full,s_empty;

void* producer(void* arg){
	int i = 0;
	while(1){
		sem_wait(&s_empty);
		value[i%6] = beginnum++;
		printf("--%s--%lu----make:%d\n",__FUNCTION__,pthread_self(),value[i%6]);
		i++;
		sem_post(&s_full);

		usleep(rand()%30000);
	}
	return NULL;
}

void* consumer(void* arg){
	int i = 0;
	while(1){
		sem_wait(&s_full);
		printf("--%s--%lu----get:%d\n",__FUNCTION__,pthread_self(),value[i%6]);
		i++;
		sem_post(&s_empty);

		usleep(rand()%50000);
	}
	return NULL;
}
  
int main(int argc,char** argv){

	sem_init(&s_full,0,0);
	sem_init(&s_empty,0,6);

	pthread_t threads[4];
	pthread_create(&threads[0],NULL,producer,NULL);
	pthread_create(&threads[1],NULL,consumer,NULL);
	//pthread_create(&threads[2],NULL,consumer,NULL);
	//pthread_create(&threads[3],NULL,producer,NULL);

	for(int i = 0; i < 2; i++){
		pthread_join(threads[i],NULL);
	}

	sem_destroy(&s_full);
	sem_destroy(&s_empty);

	return 0;
}

