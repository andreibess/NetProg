CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -std=c11
TARGET  = echo_client
SRC     = echo_client.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)

.PHONY: all clean
