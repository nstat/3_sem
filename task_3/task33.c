#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <sys/shm.h>

#define name1 "/my_sem1"
#define name2 "/my_sem2"

int main(int argc, char * argv[]) {
	sem_t * sem1, * sem2;
	if ((sem1 = sem_open(name1, O_CREAT, 0777, 1)) == SEM_FAILED) {
		perror("sem_open()");
		return -1;
	}
	if ((sem2 = sem_open(name2, O_CREAT, 0777, 0)) == SEM_FAILED) {
		perror("sem_open()");
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
	int MEM_SIZE = atoi(argv[3]);
	char * mem = (char*)mmap(0, MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);
	int status;
	pid_t pid = fork();
  	if (pid < 0) {
		perror("fork()");
		return -1;
	}
	else if (pid == 0) {//receiver
		int fdout = open(argv[2], O_CREAT | O_RDWR, S_IREAD | S_IWRITE);
		struct timespec start, stop;
		double accum;
		char * arr = (char *)malloc(MEM_SIZE);
		if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
			perror("clock_gettime()");
			return -1;
		}
		while (1) {
			sem_wait(sem2);
			if (fl[0] < 0) {
				sem_post(sem1);
				break;
			}
			else if (fl[0] > 0) {
				write(fdout, mem, fl[0]);
				sem_post(sem1);
				break;
			}
			write(fdout, mem, MEM_SIZE);
			sem_post(sem1);
		}
		if (clock_gettime(CLOCK_MONOTONIC, &stop) == -1) {
			perror("clock_gettime()");
			return -1;
		}
		accum = (stop.tv_sec - start.tv_sec) + (double)(stop.tv_nsec - start.tv_nsec) / 1.0e9;
		FILE * fdt = fopen("time.txt", "a");
		fprintf(fdt, "mmap\tr: %d\t%lf\n", MEM_SIZE, accum);
		fclose(fdt);
		exit(0);
	}
  	else {//sender		
		int fd = open(argv[1], O_CREAT | O_RDONLY);
		if (fd < 0) { 
			perror("open()");
			return -1;
		}
		struct timespec start, stop;
		double accum;
		if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
			perror("clock_gettime()");
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
		if (clock_gettime(CLOCK_MONOTONIC, &stop) == -1) {
			perror("clock_gettime()");
			return -1;
		}
		accum = (stop.tv_sec - start.tv_sec) + (double)(stop.tv_nsec - start.tv_nsec) / 1.0e9;
		close(fd);
		FILE * fdt = fopen("time.txt", "a");
		fprintf(fdt, "mmap\ts: %d\t%lf\n", MEM_SIZE, accum);
		fclose(fdt);
		waitpid(pid, &status, 0);
	}
	munmap(mem, MEM_SIZE);
	if (shmdt(fl) < 0) {
		perror("shmdt()");
		return -1;
	}
	sem_unlink(name1);
	sem_unlink(name2);
	return 0;
}
