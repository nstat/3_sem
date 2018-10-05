#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

int main(int argc, char *argv[]) {
	key_t key = 459;
	int size = 4096;
	int fd = open(argv[1], O_CREAT | O_RDONLY);
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
	read(fd, mem, size);
	if (shmdt(mem) < 0) {
		printf("shmdt() error\n");
		exit(-1);
	}
	clock_t t2 = clock();
	close(fd);
	FILE * fdt = fopen("time.txt", "a");
	fprintf(fdt, "s: %d\t%lf\n", size, (t2 - t1) / (double)CLOCKS_PER_SEC);
	fclose(fdt);
	return 0;
}
