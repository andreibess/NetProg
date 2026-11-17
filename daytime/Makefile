CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -std=c11
TARGET  = daytime_client
SRC     = daytime_client.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)

.PHONY: all clean
