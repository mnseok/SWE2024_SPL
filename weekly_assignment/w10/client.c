#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#define MAXLINE 80

int main (int argc, char *argv[]) {
	int n, cfd, fd, nbytes;
	struct hostent *h;
	struct sockaddr_in saddr;
	char buf[MAXLINE];
	char *host = argv[1];
	int port = atoi(argv[2]);
    char filename[MAXLINE];

	scanf("%s", filename);

    if ((cfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket() failed");
        exit(1);
    }

    if ((h = gethostbyname(host)) == NULL) {
        fprintf(stderr, "gethostbyname() failed\n");
        exit(2);
    }

    memset((char *)&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    memcpy((char *)&saddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
    saddr.sin_port = htons(port);
	printf("Connecting to %s (%s):%d\n", host, inet_ntoa(saddr.sin_addr), port);

    if (connect(cfd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
        perror("connect() failed");
        exit(3);
    }

    // Send the filename to the server
    if (write(cfd, filename, strlen(filename)) < 0) {
        perror("write() failed");
        exit(4);
    }

	fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	printf("fd = %d\n", fd);
	if (fd < 0) {
		printf("Failed to open file %s\n", filename);
		perror("open() failed");
		exit(5);
	}

    // Receive file content from the server
    printf("Received content of %s:\n", filename);
    while ((nbytes = read(cfd, buf, MAXLINE)) > 0) {
		printf("%s\n", buf);
        write(fd, buf, nbytes);  // Print content to the console
    }

    close(cfd);
    return 0;
}
