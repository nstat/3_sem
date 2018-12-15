#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/select.h>

#define SIZE 256
#define FILES 256

struct fifoinfo {
	char fifo1[SIZE];
	char fifo2[SIZE];
};

struct fdinfo {
	int fd1;
	int fd2;
};

const char files[FILES][SIZE] = {"file0.txt", "file1.txt", "file2.txt", "file3.txt", "file4.txt", "file5.txt", "file6.txt", "file7.txt", "file8.txt", "file9.txt", "file10.txt"};

int main() {
	int i = 0;
	mknod("server", S_IFIFO | 0666, 0);
	int fd = open("server", O_RDWR);
	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);

	struct fifoinfo rec[64];			//path to fifo
	struct fdinfo reg[64];				//descriptor for fifo

	int cl_num = 0;					
	fd_set des;
	struct timeval tv;

	while (1) {
		char buf[SIZE];
		int r = read(fd, buf, SIZE);
		if (r > -1) {
			printf("Received message\n");
			char token[SIZE];
			strcpy(token, strtok(buf, " "));
			strcpy(rec[cl_num].fifo1, strtok(NULL, " "));
			strcpy(rec[cl_num].fifo2, strtok(NULL, " "));

			reg[cl_num].fd1 = open(rec[cl_num].fifo1, O_RDWR);
			reg[cl_num].fd2 = open(rec[cl_num].fifo2, O_RDWR);
			flags = fcntl(reg[cl_num].fd1, F_GETFL, 0);
			fcntl(reg[cl_num].fd1, F_SETFL, flags | O_NONBLOCK);

			strcpy(buf, "Registered");
			printf("%s: %d\n", buf, cl_num);
			write(reg[cl_num].fd2, buf, SIZE);
			++cl_num;
		}

		if (cl_num > 0) {
			tv.tv_sec = 0;
			tv.tv_usec = 0;
			FD_ZERO(&des);
			int vol;
			for (i = 0; i < cl_num; ++i) {
				FD_SET(reg[i].fd1, &des);
				vol += reg[i].fd1;
			}
			int ret = select(vol + 1, &des, NULL, NULL, &tv);
			if (ret > 0) {
				for (i = 0; i < cl_num; ++i) {
					if (FD_ISSET(reg[i].fd1, &des)) {
						char buf3[SIZE];
						int err = 0;
						while (err <= 0) {
							err = read(reg[i].fd1, buf3, SIZE);
						}
						printf("Received message from %d\n", i);
						char tok[SIZE];
						strcpy(tok, strtok(buf3, " "));
						if (!strcmp(tok, "GET")) {
							strcpy(tok, strtok(NULL, " "));
							write(reg[i].fd2, files[atoi(tok)], SIZE); 							}
					}
				}
			}
		}
	}
	return 0;
}
