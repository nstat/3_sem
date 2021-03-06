#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <semaphore.h>
#include <errno.h>

#define name1 "/my_sem1"
#define name2 "/my_sem2"

int main(int argc, char *argv[]) {
	sem_t * sem1;
	sem_t * sem2;
	if ((sem1 = sem_open(name1, O_CREAT, 0777, 0)) == SEM_FAILED) {
		perror("sem_open()");
		return -1;
	}
	if ((sem2 = sem_open(name2, O_CREAT, 0777, 0)) == SEM_FAILED) {
		perror("sem_open()");
		return -1;
	}

	int MEM_SIZE = atoi(argv[2]);
	key_t key = 2048 + MEM_SIZE;
	int fd = open(argv[1], O_CREAT, 0777);
	if (fd < 0) {
		perror("open()");
		return -1;
	}

	key_t kfl = 11;
	int shfl;
	if ((shfl = shmget(kfl, 1, IPC_CREAT | 00666)) < 0) {
		perror("shmget()");
		return -1;
	}
	int * fl;
	if ((fl = (int *)shmat(shfl, NULL, 0)) == (int *)(-1)) {
		perror("shmat()");
		return -1;
	}
	fl[0] = 0;

	sem_wait(sem1);
	
	struct timespec start, stop;
	double accum;
	if (clock_gettime(CLOCK_MONOTONIC, &start) == -1 ) {
		perror("clock_gettime()");
		return -1;
	}

	int shmd;
	if ((shmd = shmget(key, MEM_SIZE, IPC_CREAT | 00666)) < 0) {
		perror("shmget()");
		return -1;
	}

	char * mem;
	if ((mem = (char *)shmat(shmd, NULL, 0)) == (char *)(-1)) {
		perror("shmat()");
		return -1;
	}

	int r;
	while (1) {
		sem_wait(sem1);
		r = read(fd, mem, MEM_SIZE);
		if (r <= 0) {
			fl[0] = -1;
			sem_post(sem2);
			break;
		}
		else if (r < MEM_SIZE) {
			fl[0] = r;
			sem_post(sem2);
			break;
		}
		sem_post(sem2);
	}

	if (shmdt(mem) < 0) {
		perror("shmdt()");
		return -1;
	}
	
	if (clock_gettime(CLOCK_MONOTONIC, &stop) == -1 ) {
		perror("clock_gettime()");
		return -1;
	}
	accum = (stop.tv_sec - start.tv_sec) + (double)(stop.tv_nsec - start.tv_nsec) / 1.0e9;
	
	FILE * fdt = fopen("time.txt", "a");
	fprintf(fdt, "shmem\ts: %d\t%lf\n", MEM_SIZE, accum);
	fclose(fdt);
	close(fd);
	if (shmdt(fl) < 0) {
		perror("shmdt()");
		return -1;
	}	
	return 0;
}
