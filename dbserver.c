#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define DB_HOST "127.0.0.1"
#define DB_USER "username"
#define DB_PASS "password"
#define DB_NAME "musicDB"
#define DB_PORT 3306
#define SERVER_PORT 8081
#define BUFFER_SIZE 1024

MYSQL *conn;

void connect_to_database() {
    conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        exit(EXIT_FAILURE);
    }

    if (mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, DB_PORT, NULL, 0) == NULL) {
        fprintf(stderr, "mysql_real_connect() failed\n");
        mysql_close(conn);
        exit(EXIT_FAILURE);
    }
}

void close_database_connection() {
    mysql_close(conn);
}

void fetch_mp3_files(char *responseBuffer) {
    char query[] = "SELECT fileName FROM mp3Files";
    if (mysql_query(conn, query)) {
        fprintf(stderr, "SELECT fileName FROM mp3Files failed. Error: %s\n", mysql_error(conn));
        return;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (result == NULL) {
        fprintf(stderr, "mysql_store_result() failed. Error: %s\n", mysql_error(conn));
        return;
    }

    int num_fields = mysql_num_fields(result);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        for(int i = 0; i < num_fields; i++) {
            strcat(responseBuffer, row[i] ? row[i] : "NULL");
            strcat(responseBuffer, "\n");
        }
    }
    mysql_free_result(result);
}

void *handle_client(void *client_sock) {
    int client_socket = *(int *)client_sock;
    char buffer[BUFFER_SIZE];
    char responseBuffer[BUFFER_SIZE];

    memset(buffer, 0, BUFFER_SIZE);
    read(client_socket, buffer, sizeof(buffer) - 1);

    if (strcmp(buffer, "GET MP3 LIST") == 0) {
        memset(responseBuffer, 0, BUFFER_SIZE);
        fetch_mp3_files(responseBuffer);
        send(client_socket, responseBuffer, strlen(responseBuffer), 0);
    }
    // Handle other requests as needed...

    close(client_socket);
    free(client_sock);
    return NULL;
}

int main() {
    connect_to_database();

    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len = sizeof(client_address);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address));
    listen(server_socket, 5);

    printf("DB Server started and listening on port %d\n", SERVER_PORT);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
        pthread_t client_thread;

        int *new_sock = malloc(sizeof(int));
        *new_sock = client_socket;

        pthread_create(&client_thread, NULL, handle_client, new_sock);
    }

    close(server_socket);
    close_database_connection();
    return 0;
}
