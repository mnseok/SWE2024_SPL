#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 1024
#define NUM_SEATS 256

// Query data type from client.c
typedef struct _query {
    int user;
    int action;
    int data;
} query;

int server_running = 1;
int server_socket, client_socket;
// Data Structures
int seats[NUM_SEATS]; // Array to manage seat reservations
int user_login_status[MAX_CLIENTS]; // User login status (-1: logged out or unregistered, else: logged in)
int user_passcodes[MAX_CLIENTS];    // -1: unregistered
pthread_mutex_t login_mutex[MAX_CLIENTS]; // Mutex for each user
pthread_mutex_t seat_mutex[NUM_SEATS]; // Mutex for each seat

int validate_user(int user, int client_socket);
int handle_login(int user, int passcode, int client_socket);
int handle_reserve(int user, int seat);
int handle_check(int user);
int handle_cancel(int user, int seat);
int handle_logout(int user, int client_socket);

// Signal handler for safe termination
void handle_sigint(int sig) {
    printf("\n[SERVER] Terminating safely...\n");
    server_running = 0;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (user_login_status[i] != -1) {
            close(user_login_status[i]);
        }
    }
    // mutex destroy
    for (int i = 0; i < MAX_CLIENTS; i++) {
        pthread_mutex_destroy(&login_mutex[i]);
    }
    for (int i = 0; i < NUM_SEATS; i++) {
        pthread_mutex_destroy(&seat_mutex[i]);
    }
    
    close(server_socket);
    exit(0);
}

// Client handling function
void *client_handler(void *arg) {
    int client_socket = *(int *)arg;
    int response = -1;
    free(arg);
    printf("[SERVER] New client connected.\n");

    while (server_running) {
        query q;
        if (recv(client_socket, &q, sizeof(q), 0) <= 0) break;

        printf("[CLIENT] User: %d, Action: %d, Data: %d\n", q.user, q.action, q.data);

        if (q.action == 0 && q.user == 0 && q.data == 0) {
            send(client_socket, seats, sizeof(seats), 0); // Send seat array to client
            break; // Exit the loop
        }

        int response = -1; // Default failure response
        // TODO: Add action handling logic (Log in, Reserve, Check, Cancel, Logout)
        switch (q.action) {
            case 1: response = handle_login(q.user, q.data, client_socket); break; // Log in 
            case 2: case 3: case 4: case 5:
                if (validate_user(q.user, client_socket) == 1) {
                    switch (q.action) {
                        case 2: response = handle_reserve(q.user, q.data); break;
                        case 3: response = handle_check(q.user); break;
                        case 4: response = handle_cancel(q.user, q.data); break;
                        case 5: response = handle_logout(q.user, client_socket); break;
                    }
                } else {
                    response = -1; // User validation failed
                }
                break;
            default:
            response = -1; // Invalid action
        }


        // Respond to client
        send(client_socket, &response, sizeof(response), 0);
    }

    printf("[SERVER] Client disconnected.\n");
    close(client_socket);
    return NULL;
}

int validate_user(int user, int client_socket) {
    if (user < 0 || user >= MAX_CLIENTS) {
        return -1; // Invalid user ID
    }

    pthread_mutex_lock(&login_mutex[user]);
    if (user_login_status[user] == -1) {
        return -1; // User not logged in
    }

    if (user_login_status[user] != client_socket) {
        return -1; // Different client logged in
    }
    pthread_mutex_unlock(&login_mutex[user]);

    return 1; // User is valid
}

// 로그인 핸들링 함수
int handle_login(int user, int passcode, int client_socket) {
    int response = -1; // 기본 응답: 실패


    if (user < 0 || user >= MAX_CLIENTS) {
        response = -1; // 유효하지 않은 사용자 ID
    } 
    
    // lock mutex
    pthread_mutex_lock(&login_mutex[user]);

    if (user_login_status[user] != -1) {
        // 다른 클라이언트에서 이미 로그인됨
        response = -1;
    } else if (user_passcodes[user] == -1) {
        // 첫 로그인 시 등록
        user_passcodes[user] = passcode;
        user_login_status[user] = client_socket;
        response = 1;
        printf("[SERVER] User %d registered and logged in.\n", user);
    } else if (user_passcodes[user] == passcode) {
        // 올바른 패스코드로 로그인
        user_login_status[user] = client_socket;
        response = 1;
        printf("[SERVER] User %d logged in.\n", user);
    } else {
        // 패스코드 불일치
        response = -1;
    }

    // unlock mutex
    pthread_mutex_unlock(&login_mutex[user]);
    return response;
}

int handle_reserve(int user, int seat) {
    int response = -1;

    if (seat < 0 || seat >= NUM_SEATS) {
        response = -1; // Invalid seat number
    } else {
        // Lock the mutex for the requested seat
        pthread_mutex_lock(&seat_mutex[seat]);

        if (seats[seat] == -1) {
            seats[seat] = user; // Reserve the seat
            response = seat;
            printf("[SERVER] Seat %d reserved by User %d.\n", seat, user);
        } else {
            response = -1; // Seat already reserved
        }

        // Unlock the mutex
        pthread_mutex_unlock(&seat_mutex[seat]);
    }

    return response;
}

int handle_check(int user) {
    int response = -1;
    
    // Check for the seat reserved by the user
    for (int i = 0; i < NUM_SEATS; i++) {
        pthread_mutex_lock(&seat_mutex[i]); // Protect seat checking
        if (seats[i] == user) {
            response = i; // Return the seat number
            pthread_mutex_unlock(&seat_mutex[i]);
            return response;
        }
        pthread_mutex_unlock(&seat_mutex[i]);
    }

    return response; // -1 if no reservation found
}

int handle_cancel(int user, int seat) {
    int response = -1;

    pthread_mutex_lock(&seat_mutex[seat]); // Lock the seat mutex
    if (seats[seat] == user) { // Check if the user has the seat reserved
        seats[seat] = -1; // Cancel the reservation
        response = seat;
        printf("[SERVER] Reservation for Seat %d canceled by User %d.\n", seat, user);
    }
    pthread_mutex_unlock(&seat_mutex[seat]); // Unlock the seat mutex

    return response; // 예약된 좌석이 없는 경우 -1 반환
}

int handle_logout(int user, int client_socket) {
    int response = -1;

    pthread_mutex_lock(&login_mutex[user]);
    if (user_login_status[user] == client_socket) {
        user_login_status[user] = -1; // 로그아웃 처리
        response = 1;
        printf("[SERVER] User %d logged out.\n", user);
    } else {
        response = -1; // 다른 클라이언트에서 로그인 중
    }
    pthread_mutex_unlock(&login_mutex[user]);

    return response;
}

int main(int argc, char *argv[]) {
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    pthread_t tid;
    
    if (argc < 3) {
        printf("Usage: %s <server_ip> <port_number>\n", argv[0]);
        exit(1);
    }

    char *server_ip = argv[1];
    int port = atoi(argv[2]);


    // Initialize seat and user data
    memset(seats, -1, sizeof(seats));
    memset(user_login_status, -1, sizeof(user_login_status));
    memset(user_passcodes, -1, sizeof(user_passcodes));


    // Signal handling
    signal(SIGINT, handle_sigint);

    // Socket creation
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        exit(1);
    }

    if (listen(server_socket, MAX_CLIENTS) == -1) {
        perror("Listen failed");
        exit(1);
    }

    printf("[SERVER] Listening on port %d...\n", port);

    // Accept clients in a loop
    while (server_running) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket == -1) {
            perror("Accept failed");
            continue;
        }

        int *new_client_socket = malloc(sizeof(int));
        *new_client_socket = client_socket;
        pthread_create(&tid, NULL, client_handler, new_client_socket);
        pthread_detach(tid); // Automatically clean up thread resources
    }

    close(server_socket);
    return 0;
}
