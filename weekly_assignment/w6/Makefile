TARGET=vsd
CC=gcc
CFLAGS=-Wall
OBJECTS=$(TARGET).o

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^
$(TARGET).o: $(TARGET).c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f $(OBJECTS) $(TARGET)
