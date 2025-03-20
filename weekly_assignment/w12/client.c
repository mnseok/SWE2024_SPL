#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netdb.h> // gethostbyname 추가

#define BUFFER_SIZE 1024

void *receive_messages(void *socket_fd) {
    int sock = *(int *)socket_fd;
    char buffer[BUFFER_SIZE];
    int bytes_read;

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        bytes_read = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytes_read <= 0) {
            printf("Disconnected from server.\n");
            close(sock);
            exit(0);
        }
        printf("%s", buffer);
        fflush(stdout);
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE];
    char name[50];
    pthread_t recv_thread;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // 소켓 생성
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[2]));

    // IP 주소 변환 및 설정
    if (strcmp(argv[1], "localhost") == 0) {
        struct hostent *host = gethostbyname(argv[1]);
        if (host == NULL) {
            perror("Error resolving localhost");
            exit(EXIT_FAILURE);
        }
        memcpy(&server_address.sin_addr, host->h_addr, host->h_length);
    } else if (inet_pton(AF_INET, argv[1], &server_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // 서버에 연결
    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to the server.\n");

    // 사용자 이름 입력
    printf("Insert your name: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0'; // 개행 문자 제거

    // 서버로 이름 전송
    send(sock, name, strlen(name), 0);

    // 수신 스레드 생성
    if (pthread_create(&recv_thread, NULL, receive_messages, &sock) != 0) {
        perror("Failed to create thread");
        exit(EXIT_FAILURE);
    }

    // 메시지 전송 루프
    while (1) {
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0'; // 개행 문자 제거

        if (strcmp(buffer, "quit") == 0) {
            close(sock); // 소켓 종료
            exit(0); // 프로그램 종료
        }

        send(sock, buffer, strlen(buffer), 0);
    }

    close(sock);
    return 0;
}
