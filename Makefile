# CC := gcc
# # macOS LDFLAGS configs:
# LDFLAGS := -L/opt/homebrew/opt/openssl@3/lib -lssl -lcrypto
# # default OS LDFLAGS configs:
# # LDFLAGS := -lssl -lcrypto
# UNAME := $(shell uname)

# ifeq ($(UNAME), Darwin)
# # macOS CFLAGS configs:
# CFLAGS := -I/opt/homebrew/opt/openssl@3/include
# # default OS CFLAGS configs:
# # CFLAGS := -I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib
# endif
# all: client server

# client: client.o
# 	$(CC) $(CFLAGS) -o client client.o $(LDFLAGS)

# client.o: client.c
# 	$(CC) $(CFLAGS) -c client.c

# server: server.o
# 	$(CC) $(CFLAGS) -o server server.o $(LDFLAGS)

# server.o: server.c
# 	$(CC) $(CFLAGS) -c server.c

# clean:
# 	rm -f server server.o client client.o

CC := gcc

# macOS-specific configurations:
LDFLAGS := -L/opt/homebrew/opt/openssl@3/lib -lssl -lcrypto
CFLAGS := -I/opt/homebrew/opt/openssl@3/include

# Explicitly adding MySQL paths:
LDFLAGS += -L/opt/homebrew/opt/mysql-client/lib -lmysqlclient
CFLAGS += -I/opt/homebrew/opt/mysql-client/include

all: client server dbserver

client: client.o
	$(CC) $(CFLAGS) -o client client.o $(LDFLAGS)

client.o: client.c
	$(CC) $(CFLAGS) -c client.c

server: server.o
	$(CC) $(CFLAGS) -o server server.o $(LDFLAGS)

server.o: server.c
	$(CC) $(CFLAGS) -c server.c

dbserver: dbserver.o
	$(CC) $(CFLAGS) -o dbserver dbserver.o $(LDFLAGS)

dbserver.o: dbserver.c
	$(CC) $(CFLAGS) -c dbserver.c

clean:
	rm -f server server.o client client.o
