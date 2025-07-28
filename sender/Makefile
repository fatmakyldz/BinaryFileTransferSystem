CC = gcc
CFLAGS = -Wall -O2 -I/opt/homebrew/opt/openssl@3/include -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/opt/openssl@3/lib -lssl -lcrypto -lzmq

TARGET = sender
SRC = main.c

all:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
