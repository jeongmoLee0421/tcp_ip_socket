#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

void* read(void* arg);
void* accu(void* arg);
static sem_t sem_one;
static sem_t sem_two;
static int num;

int main(int argc, char* argv[]){
    pthread_t id_t1, id_t2;
    
    // int sem_init(sem_t* sem, int pshared, unsigned int value);
    sem_init(&sem_one, 0, 0);
    sem_init(&sem_two, 0, 1);
    
    pthread_create(&id_t1, NULL, read, NULL);
    pthread_create(&id_t2, NULL, accu, NULL);
    
    pthread_join(id_t1, NULL);
    pthread_join(id_t2, NULL);
    
    // int sem_destroy(sem_t* sem);
    sem_destroy(&sem_one);
    sem_destroy(&sem_two);
}

void* read(void* arg){
    int i;
    for (i=0; i<5; i++){
        fputs("Input num: ", stdout);
        
        sem_wait(&sem_two); // sem_two -= 1
        scanf("%d", &num);
        sem_post(&sem_one); // sum_one += 1
    }
    return NULL;
}

void* accu(void* arg){
    int sum = 0, i;
    for (i=0; i<5; i++){
        sem_wait(&sem_one); // sem_one -= 1
        sum += num;
        sem_post(&sem_two); // sem_two += 1
    }
    printf("Result: %d\n", sum);
    return NULL;
}

/*
scenario

1. read
2. accu

semaphore control the flow of execution.
*/
