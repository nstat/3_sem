#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

int main() {
	key_t key = 459;
	int size = 4096;
	int fd = open("testrec.txt", O_CREAT | O_RDWR, S_IREAD | S_IWRITE);
	if (fd < 0) {
		printf("open() error\n");
		exit(-1);
	}
	clock_t t1 = clock();
	int shmd;
	if ((shmd = shmget(key, size, IPC_CREAT | 00666)) < 0) {
		printf("shmget() error\n");
		exit(-1);
	}
	char * mem;
	if ((mem = (char *)shmat(shmd, NULL, 0)) == (char *)(-1)) {
		printf("shmat() error\n");
		exit(-1);
	}
	write(fd, mem, size);
	if (shmdt(mem) < 0) {
		printf("shmdt() error\n");
		exit(-1);
	}
	clock_t t2 = clock();
	close(fd);
	FILE * fdt = fopen("time.txt", "a");
	fprintf(fdt, "r: %d\t%lf\n", size, (t2 - t1) / (double)CLOCKS_PER_SEC);
	fclose(fdt);
	return 0;
}

