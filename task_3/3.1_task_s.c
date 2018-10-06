#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

#define MEM_SIZE 4096

int main(int argc, char *argv[]) {
	key_t key = 459;
	int fd = open(argv[1], O_CREAT | O_RDONLY);
	if (fd < 0) {
		printf("open() error\n");
		exit(-1);
	}
	clock_t t1 = clock();
	int shmd;
	if ((shmd = shmget(key, MEM_SIZE, IPC_CREAT | 00666)) < 0) {
		printf("shmget() error\n");
		exit(-1);
	}
	char * mem;
	if ((mem = (char *)shmat(shmd, NULL, 0)) == (char *)(-1)) {
		printf("shmat() error\n");
		exit(-1);
	}
	read(fd, mem, MEM_SIZE);
	if (shmdt(mem) < 0) {
		printf("shmdt() error\n");
		exit(-1);
	}
	clock_t t2 = clock();
	close(fd);
	FILE * fdt = fopen("time.txt", "a");
	fprintf(fdt, "shmem\ts: %d\t%lf\n", MEM_SIZE, (t2 - t1) / (double)CLOCKS_PER_SEC);
	fclose(fdt);
	return 0;
}
