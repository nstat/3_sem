#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/shm.h>

#define name1 "/my_sem1"
#define name2 "/my_sem2"

#define MEM_SIZE 4096

typedef struct msgbuf{
	long mtype;
	char mtext[MEM_SIZE];
} msg_buf;

int main(int argc, char * argv[]) {
	sem_t * sem1, * sem2;
	key_t kfl = 11, key = 4096 + MEM_SIZE;
	int shfl, fd;
	if ((sem1 = sem_open(name1, 0)) == SEM_FAILED) {
		perror("sem_open()");
		return -1;
	}
	if ((sem2 = sem_open(name2, 0)) == SEM_FAILED) {
		perror("sem_open()");
		return -1;
	}
	if ((shfl = shmget(kfl, 1, IPC_CREAT | 00666)) < 0) {
		perror("shmget()");
		return -1;
	}	
	int * fl;
	if ((fl = (int *)shmat(shfl, NULL, 0)) == (int *)(-1)) {
		perror("shmat()");
		return -1;
	}
	msg_buf mem;
	int file = open(argv[1], O_CREAT | O_RDWR, S_IREAD | S_IWRITE);
	if (file < 0) {
		perror("open()");
		return -1;
	}
	if ((fd = msgget(key, IPC_CREAT | 0664)) < 0) {
		perror("msgget()");
		return -1;
	}
	sem_wait(sem2);
	struct timespec start, stop;
	double accum;
	if(clock_gettime(CLOCK_MONOTONIC, &start) == -1 ) {
      		perror("clock_gettime()");
      		return EXIT_FAILURE;
    	}
	while (1) {
		sem_wait(sem2);
		if (fl[0] < 0) {
			sem_post(sem1);
			break;
		}
		if ((msgrcv(fd, &mem, MEM_SIZE, 0, 0)) < 0) {
			perror("msgrcv()");
			return -1;
		}
		if (fl[0] > 0) {
			write(file, mem.mtext, fl[0]);
			sem_post(sem1);
			break;
		}
		write(file, mem.mtext, MEM_SIZE);
		sem_post(sem1);
	}
	if (clock_gettime(CLOCK_MONOTONIC, &stop) == -1 ) {
      		perror("clock_gettime()");
      		return EXIT_FAILURE;
    	}
	accum = (stop.tv_sec - start.tv_sec) + (double)(stop.tv_nsec - start.tv_nsec) / 1.0e9;
        
	close(file);
	FILE * fdt = fopen("time.txt", "a");
        fprintf(fdt, "msg\tr: %d\t%lf\n", MEM_SIZE, accum);
        fclose(fdt);
	sem_unlink(name1);
	sem_unlink(name2);
	return 0;
}
