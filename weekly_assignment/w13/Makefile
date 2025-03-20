# Compiler and flags
CC = gcc
CFLAGS = -Wall -pthread

# Target executable
TARGET = main

# Source files
SRC = main.c

# Build target
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Clean target
clean:
	rm -f $(TARGET)

# Run target
run: $(TARGET)
	./$(TARGET)
