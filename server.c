#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define DB_SERVER_PORT 3307

SSL_CTX *init_server_ctx()
{
    SSL_CTX *ctx;

    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    ctx = SSL_CTX_new(TLS_server_method());

    if (!SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM) ||
        !SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM))
    {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void handle_db_request(int client_socket, SSL *ssl)
{
    int db_socket;
    struct sockaddr_in dbserver_address;
    char buffer[BUFFER_SIZE];

    db_socket = socket(AF_INET, SOCK_STREAM, 0);
    memset(&dbserver_address, 0, sizeof(dbserver_address));
    dbserver_address.sin_family = AF_INET;
    dbserver_address.sin_port = htons(DB_SERVER_PORT);
    dbserver_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(db_socket, (struct sockaddr *)&dbserver_address, sizeof(dbserver_address)) != 0)
    {
        perror("Could not connect to dbserver");
        exit(EXIT_FAILURE);
    }

    char request[] = "LIST";
    send(db_socket, request, strlen(request), 0);
    memset(buffer, 0, sizeof(buffer));
    recv(db_socket, buffer, sizeof(buffer) - 1, 0);

    SSL_write(ssl, buffer, strlen(buffer));
    memset(buffer, 0, sizeof(buffer));
    SSL_read(ssl, buffer, sizeof(buffer) - 1);

    char link_request[BUFFER_SIZE];
    snprintf(link_request, sizeof(link_request), "GET MP3 LINK %s", buffer);

    send(db_socket, link_request, strlen(link_request), 0);
    memset(buffer, 0, sizeof(buffer));
    recv(db_socket, buffer, sizeof(buffer) - 1, 0);

    SSL_write(ssl, buffer, strlen(buffer));
    close(db_socket);
}

void *handle_client(void *arg)
{
    int client_socket = *(int *)arg;
    SSL *ssl = SSL_new(init_server_ctx());
    SSL_set_fd(ssl, client_socket);

    if (SSL_accept(ssl) <= 0)
    {
        ERR_print_errors_fp(stderr);
    }
    else
    {
        handle_db_request(client_socket, ssl);
    }

    SSL_free(ssl);
    close(client_socket);
    free(arg);
    return NULL;
}

int main()
{
    int server_socket;
    struct sockaddr_in server_address;
    socklen_t addr_len = sizeof(server_address);

    OpenSSL_add_all_algorithms();
    SSL_library_init();

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address));
    listen(server_socket, 5);
    printf("Server started and listening on port %d\n", PORT);

    while (1)
    {
        int client_socket = accept(server_socket, (struct sockaddr *)&server_address, &addr_len);
        int *new_sock = malloc(sizeof(int));
        *new_sock = client_socket;

        pthread_t client_thread;
        pthread_create(&client_thread, NULL, handle_client, new_sock);
        pthread_detach(client_thread);
    }

    close(server_socket);
    return 0;
}
