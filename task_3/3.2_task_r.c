#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>

#define MEM_SIZE 4096

int main(int argc, char * argv[]) {
	key_t key = 1234;
	int fd;
	char mem[MEM_SIZE];
	int file = open("received.txt", O_CREAT | O_RDWR, S_IREAD | S_IWRITE);
	if (file < 0) {
		printf("open() error\n");
		exit(-1);
	}
	clock_t t1 = clock();
	if ((fd = msgget(key, IPC_CREAT | 0664)) < 0) {
		printf("msgget() error \n");
		exit(-1);
	}
	if ((msgrcv(fd, mem, MEM_SIZE, 0, 0)) < 0) {
		printf("msgrcv() error\n");
		exit(-1);
	}
	clock_t t2 = clock();
	write(file, mem, MEM_SIZE);
        close(file);
	FILE * fdt = fopen("time.txt", "a");
        fprintf(fdt, "msg\ts: %d\t%lf\n", MEM_SIZE, (t2 - t1) / (double)CLOCKS_PER_SEC);
        fclose(fdt);
	return 0;
}
