#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

int main() {
	sem_t * sem1 = sem_open("/my_sem1", 0);
	sem_t * sem2 = sem_open("/my_sem2", 0);
	sem_post(sem1);
	sem_post(sem2);
	sem_post(sem1);
	return 0;
}
