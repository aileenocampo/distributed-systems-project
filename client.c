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

#define PORT 8080
#define BUFFER_SIZE 1024

SSL_CTX *init_client_ctx()
{
    SSL_CTX *ctx;

    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    ctx = SSL_CTX_new(TLS_client_method());

    if (!SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM) ||
        !SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM))
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

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    printf("Attempting to connect to server on port %d...\n", PORT);

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) != 0)
    {
        perror("Could not connect to server");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Socket connection established with server.\n");
    }

    ssl = SSL_new(init_client_ctx());
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
            printf("Available MP3 files:\n%s", buffer);

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
    close(client_socket);
    return 0;
}
