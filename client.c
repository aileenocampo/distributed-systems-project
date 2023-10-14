#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <arpa/inet.h>

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

// ... [rest of the includes and init_client_ctx function remain unchanged]

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

    // Notify the user about the port the client is trying to connect to
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

        int bytes = SSL_read(ssl, buffer, sizeof(buffer));
        printf("Received data: %s\n", buffer);
        if (bytes > 0)
        {
            printf("Available MP3 files:\n%s", buffer);
        }

        char choice[BUFFER_SIZE];
        printf("Enter the name of the MP3 file you want to download: ");
        fgets(choice, sizeof(choice), stdin);
        choice[strcspn(choice, "\n")] = 0;

        SSL_write(ssl, choice, strlen(choice));

        FILE *file = fopen(choice, "wb");
        if (file)
        {
            while ((bytes = SSL_read(ssl, buffer, sizeof(buffer))) > 0)
            {
                fwrite(buffer, 1, bytes, file);
            }
            fclose(file);
            printf("File '%s' downloaded successfully!\n", choice);
        }
        else
        {
            printf("Error saving the file locally.\n");
        }
    }

    SSL_free(ssl);
    close(client_socket);
    return 0;
}