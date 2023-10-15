CC := gcc

# macOS-specific configurations:
LDFLAGS := -L/opt/homebrew/opt/openssl@3/lib -lssl -lcrypto
CFLAGS := -I/opt/homebrew/opt/openssl@3/include
LDFLAGS += -L/opt/homebrew/opt/mysql-client/lib -lmysqlclient
CFLAGS += -I/opt/homebrew/opt/mysql-client/include

# # Placeholder for other OS configurations (adjust accordingly):
# # LDFLAGS += -lmysqlclient
# # CFLAGS += -I/path/to/mysql/include

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
	rm -f server server.o client client.o dbserver dbserver.o
