TARGET=w5
CC=gcc
CFLAGS=-Wall
OBJECTS=w5.o

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^
w5.o: w5.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f $(OBJECTS) $(TARGET)
