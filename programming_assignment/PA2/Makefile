CC = gcc
CFLAGS = -pthread
SERVER_SRC = server.c
CLIENT_SRC = client.c
SERVER_OUT = server
CLIENT_OUT = client

all: server client

# 서버 빌드
server: $(SERVER_SRC)
	$(CC) $(CFLAGS) -o $(SERVER_OUT) $(SERVER_SRC)

# 클라이언트 빌드
client: $(CLIENT_SRC)
	$(CC) -o $(CLIENT_OUT) $(CLIENT_SRC)

# 정리 타겟
clean:
	rm -f $(SERVER_OUT) $(CLIENT_OUT)

.PHONY: all clean
