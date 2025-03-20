#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define MAX_NAME_LENGTH 50 // 닉네임 최대 길이
#define MAX_MESSAGE_LENGTH 950 // 메시지 최대 길이

int main(int argc, char *argv[]) {
    int server_fd, max_fd, activity, new_socket;
    int client_sockets[MAX_CLIENTS] = {0}; // 클라이언트 소켓 배열
    int client_initialized[MAX_CLIENTS] = {0}; // 닉네임 설정 여부 배열
    struct sockaddr_in address;
    char buffer[BUFFER_SIZE];
    char temp_buffer[BUFFER_SIZE]; // 임시 버퍼
    char client_names[MAX_CLIENTS][MAX_NAME_LENGTH + 1] = {{0}}; // 닉네임 저장
    fd_set readfds;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        max_fd = server_fd;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int fd = client_sockets[i];
            if (fd > 0) FD_SET(fd, &readfds);
            if (fd > max_fd) max_fd = fd;
        }

        activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("Select error");
            continue;
        }

        if (FD_ISSET(server_fd, &readfds)) {
            socklen_t addrlen = sizeof(address);
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
                perror("Accept failed");
                continue;
            }

            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = new_socket;
                    client_initialized[i] = 0; // 초기화되지 않은 상태로 설정
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int fd = client_sockets[i];
            if (FD_ISSET(fd, &readfds)) {
                int valread = read(fd, buffer, BUFFER_SIZE - 1);
                if (valread <= 0) {
                    socklen_t addrlen = sizeof(address);
                    getpeername(fd, (struct sockaddr *)&address, &addrlen);

                    // 접속 종료 메시지
                    if (client_initialized[i]) {
                        int client_count = 0;
                        for (int j = 0; j < MAX_CLIENTS; j++) {
                            if (client_sockets[j] != 0 && j != i) client_count++;
                        }

                        snprintf(temp_buffer, BUFFER_SIZE, "%s has left the chatting room. (%d users)\n", client_names[i], client_count);
                        for (int j = 0; j < MAX_CLIENTS; j++) {
                            if (client_sockets[j] != 0 && j != i) {
                                send(client_sockets[j], temp_buffer, strlen(temp_buffer), 0);
                            }
                        }
                    }

                    // 클라이언트 정리
                    close(fd);
                    client_sockets[i] = 0;
                    client_initialized[i] = 0;
                    memset(client_names[i], 0, MAX_NAME_LENGTH + 1);
                } else {
                    buffer[valread] = '\0';
                    if (!client_initialized[i]) {
                        // 닉네임 설정
                        strncpy(client_names[i], buffer, MAX_NAME_LENGTH);
                        client_names[i][MAX_NAME_LENGTH] = '\0'; // null-terminate
                        client_initialized[i] = 1;

                        // 접속 클라이언트 수 계산
                        int client_count = 0;
                        for (int j = 0; j < MAX_CLIENTS; j++) {
                            if (client_sockets[j] != 0) client_count++;
                        }

                        // 모든 클라이언트에 알림
                        snprintf(temp_buffer, BUFFER_SIZE, "%s has joined the chatting room. (%d users)\n", client_names[i], client_count);
                        for (int j = 0; j < MAX_CLIENTS; j++) {
                            if (client_sockets[j] != 0 && j != i) {
                                send(client_sockets[j], temp_buffer, strlen(temp_buffer), 0);
                            }
                        }
                    } else {
                        // 닉네임이 설정된 클라이언트의 메시지
                        buffer[MAX_MESSAGE_LENGTH] = '\0'; // 메시지 길이 제한
                        printf("got %d bytes from user %s\n", valread + 1, client_names[i]);
                        int message_length = snprintf(temp_buffer, BUFFER_SIZE, "%s: %s\n", client_names[i], buffer);
                        if (message_length >= BUFFER_SIZE) {
                            fprintf(stderr, "Warning: Truncated message for client %d\n", i);
                        }
                        for (int j = 0; j < MAX_CLIENTS; j++) {
                            if (client_sockets[j] != 0 && j != i) {
                                send(client_sockets[j], temp_buffer, strlen(temp_buffer), 0);
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}
