#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>

#define MAXLINE 80

int main (int argc, char *argv[]) {
    int n, listenfd, connfd, caddrlen, nbytes, fd;
    struct sockaddr_in saddr, caddr;
    char buf[MAXLINE];
    int port = atoi(argv[1]);

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket() failed");
        exit(1);
    }

    memset((char *)&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_port = htons(port);

    if (bind(listenfd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
        perror("bind() failed");
        exit(2);
    }

    if (listen(listenfd, 5) < 0) {
        perror("listen() failed");
        exit(3);
    }

    while (1) {
        caddrlen = sizeof(caddr);
        if ((connfd = accept(listenfd, (struct sockaddr *)&caddr, &caddrlen)) < 0) {
            perror("accept() failed");
            exit(4);
        }

        if ((n = read(connfd, buf, MAXLINE - 1)) < 0) {
            perror("read() failed");
            exit(5);
        }

        buf[n] = '\0';  // null terminate the buffer

        if ((fd = open(buf, O_RDONLY)) < 0) {
            printf("Failed to open file %s\n", buf);
            snprintf(buf, MAXLINE, "Failed to open file: %s\n", strerror(errno));
            write(connfd, buf, strlen(buf));
            close(connfd);
            continue;
        }

        // Read from file and send to client
        while ((nbytes = read(fd, buf, MAXLINE)) > 0) {
            write(connfd, buf, nbytes);
        }

        close(fd);
        close(connfd);
    }
}
