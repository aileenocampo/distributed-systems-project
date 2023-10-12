CC := gcc
# macOS LDFLAGS configs:
LDFLAGS := -L/opt/homebrew/opt/openssl@3/lib -lssl -lcrypto
# default OS LDFLAGS configs:
# LDFLAGS := -lssl -lcrypto
UNAME := $(shell uname)

ifeq ($(UNAME), Darwin)
# macOS CFLAGS configs:
CFLAGS := -I/opt/homebrew/opt/openssl@3/include
# default OS CFLAGS configs:
# CFLAGS := -I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib
endif
all: client server

client: client.o
	$(CC) $(CFLAGS) -o client client.o $(LDFLAGS)

client.o: client.c
	$(CC) $(CFLAGS) -c client.c

server: server.o
	$(CC) $(CFLAGS) -o server server.o $(LDFLAGS)

server.o: server.c
	$(CC) $(CFLAGS) -c server.c

clean:
	rm -f server server.o client client.o
