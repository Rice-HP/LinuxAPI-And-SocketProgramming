#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

int beginnum = 100;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

typedef struct _List{
	int val;
	struct _List* next;
}Node;

Node* head = NULL;

void* producer(void* arg){
	
	while(1){
		Node* temp = malloc(sizeof(Node));
		temp->val = beginnum++;

		pthread_mutex_lock(&mutex);
	
		temp->next = head;
		head = temp;
		printf("--%s--%u----create:%d\n",__FUNCTION__,pthread_self(),head->val);

		pthread_mutex_unlock(&mutex);
	
		pthread_cond_signal(&cond); //唤醒一个

		usleep(rand()%3000);
	}
	return NULL;
}

void* consumer(void* arg){
	
	Node* temp = NULL;
	
	while(1){
		pthread_mutex_lock(&mutex);
	
		while(head == NULL){
			pthread_cond_wait(&cond,&mutex);
		}
		
		while(head){
			temp = head;
			printf("--%s--%u----val:%d\n",__FUNCTION__,pthread_self(),head->val);
			head = head->next;
			free(temp);
		}

		pthread_mutex_unlock(&mutex);

		usleep(rand()%5000);
	}
	return NULL;
}
  
int main(int argc,char** argv){

	pthread_t threads[3];
	pthread_create(&threads[0],NULL,producer,NULL);
	pthread_create(&threads[1],NULL,consumer,NULL);
	pthread_create(&threads[2],NULL,consumer,NULL);

	for(int i = 0; i < 3; i++){
		pthread_join(threads[i],NULL);
	}

	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);
	return 0;
}

