#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <dirent.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define PORT 8080
#define BUFFER_SIZE 1024

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

void list_files(int client_socket, SSL *ssl)
{
    DIR *d;
    struct dirent *dir;
    char buffer[BUFFER_SIZE];

    // d = opendir(".");
    d = opendir("media");
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if (strstr(dir->d_name, ".mp3"))
            {
                // snprintf(buffer, sizeof(buffer), "%s\n", dir->d_name);
                // SSL_write(ssl, buffer, strlen(buffer));
                strncat(buffer, dir->d_name, sizeof(buffer) - strlen(buffer) - 1);
                strncat(buffer, "\n", sizeof(buffer) - strlen(buffer) - 1);
            }
        }
        SSL_write(ssl, buffer, strlen(buffer));
        closedir(d);
    }
}

void *handle_client(void *arg)
{
    int client_socket = *(int *)arg;
    SSL *ssl;

    ssl = SSL_new(init_server_ctx());
    SSL_set_fd(ssl, client_socket);

    if (SSL_accept(ssl) <= 0)
    {
        ERR_print_errors_fp(stderr);
    }
    else
    {
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, sizeof(buffer));

        list_files(client_socket, ssl);

        // ... further code to send the selected file ...
        memset(buffer, 0, sizeof(buffer));
        SSL_read(ssl, buffer, sizeof(buffer) - 1);

        char filepath[BUFFER_SIZE];
        snprintf(filepath, sizeof(filepath), "media/%.*s", (int)(BUFFER_SIZE - 7 - 1), buffer);
        FILE *file = fopen(buffer, "rb");
        // FILE *file = fopen(strcat("media/", buffer), "rb");
        if (file)
        {
            char send_buffer[BUFFER_SIZE];
            int bytes_read;

            while ((bytes_read = fread(send_buffer, 1, sizeof(send_buffer), file)) > 0)
            {
                SSL_write(ssl, send_buffer, bytes_read);
            }

            fclose(file);
        }
        else
        {
            char msg[] = "Error opening file";
            SSL_write(ssl, msg, strlen(msg));
        }
    }

    SSL_free(ssl);
    close(client_socket);
    return NULL;
}

int main()
{
    int server_socket, client_socket;
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

    // Notification that the server is listening
    printf("Server started and listening on port %d\n", PORT);

    while (1)
    {
        client_socket = accept(server_socket, (struct sockaddr *)&server_address, &addr_len);

        pthread_t client_thread;
        pthread_create(&client_thread, NULL, handle_client, &client_socket);
    }

    close(server_socket);
    return 0;
}