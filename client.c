#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_COUNT 2
#define BUFFER_SIZE 1024

const int ports[SERVER_COUNT] = {8080, 8081};
const char *server_addresses[SERVER_COUNT] = {"127.0.0.1", "127.0.0.1"}; // Using the same IP for both servers

SSL_CTX *init_client_ctx()
{
    SSL_CTX *ctx;

    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    ctx = SSL_CTX_new(TLS_client_method());

    if (ctx == NULL)
    {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    return ctx;
}

int main()
{
    int client_socket;
    struct sockaddr_in server_address;
    SSL *ssl;

    OpenSSL_add_all_algorithms();
    SSL_library_init();

    SSL_CTX *ctx = init_client_ctx(); // Initialize SSL_CTX once

    for (int i = 0; i < SERVER_COUNT; i++)
    {
        client_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (client_socket == -1)
        {
            perror("Could not create socket");
            continue; // Try the next server
        }

        // Set the timeout for the socket
        struct timeval timeout;
        timeout.tv_sec = 5; // 5 seconds timeout
        timeout.tv_usec = 0;
        setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
        setsockopt(client_socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout));

        memset(&server_address, 0, sizeof(server_address));
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(ports[i]);
        server_address.sin_addr.s_addr = inet_addr(server_addresses[i]);

        printf("Attempting to connect to server %s on port %d...\n", server_addresses[i], ports[i]);

        if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == 0)
        {
            printf("Socket connection established with server %s on port %d.\n", server_addresses[i], ports[i]);
            break;
        }
        else
        {
            perror("Could not connect to server");
            close(client_socket);
            if (i == SERVER_COUNT - 1)
            {
                printf("Failed to connect to any server. Exiting.\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, client_socket);

    if (SSL_connect(ssl) == -1)
    {
        ERR_print_errors_fp(stderr);
    }
    else
    {
        printf("SSL connection established with server.\n");

        char buffer[BUFFER_SIZE];
        memset(buffer, 0, sizeof(buffer));

        int bytes = SSL_read(ssl, buffer, sizeof(buffer) - 1);
        buffer[bytes] = '\0';
        if (bytes > 0)
        {
            printf("Available MP3 files:\n");
            char *token = strtok(buffer, "\n");
            int index = 0;
            while (token != NULL)
            {
                printf("%d: %s\n", ++index, token);
                token = strtok(NULL, "\n");
            }
        }

        int choice;
        printf("Enter the number of the MP3 file you want to download: ");
        scanf("%d", &choice);
        getchar();

        snprintf(buffer, sizeof(buffer), "%d", choice);
        SSL_write(ssl, buffer, strlen(buffer));

        memset(buffer, 0, sizeof(buffer));
        bytes = SSL_read(ssl, buffer, sizeof(buffer) - 1);
        buffer[bytes] = '\0';
        if (bytes > 0)
        {
            char currentDir[512];
            getcwd(currentDir, sizeof(currentDir));

            char command[512];
            char filename[512];

            snprintf(filename, sizeof(filename), "%s/song.mp3", currentDir);
            snprintf(command, sizeof(command), "curl -o %s -L %s", filename, buffer);
            system(command);
            printf("Downloaded song to: %s\n", filename);
        }
        else
        {
            printf("No data received from server.\n");
        }
    }

    SSL_free(ssl);
    SSL_CTX_free(ctx); // Free the SSL_CTX object
    close(client_socket);
    return 0;
}
