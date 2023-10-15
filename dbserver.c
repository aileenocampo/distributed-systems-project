#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <mysql/mysql.h>
#include <ctype.h>

#define DB_SERVER_PORT 3307
#define BUFFER_SIZE 1024
#define SERVER "127.0.0.1"
#define USER "username"
#define PASSWORD "password"
#define DATABASE "musicDB"

MYSQL *conn;

MYSQL *connect_to_database()
{
  conn = mysql_init(NULL);
  if (!conn)
  {
    fprintf(stderr, "mysql_init() failed\n");
    return NULL;
  }

  if (mysql_real_connect(conn, SERVER, USER, PASSWORD, DATABASE, 0, NULL, 0) == NULL)
  {
    fprintf(stderr, "Connection failed: %s\n", mysql_error(conn));
    return NULL;
  }

  return conn;
}

void fetch_mp3_files(char *responseBuffer)
{
  char query[] = "SELECT songName FROM mp3Files";
  if (mysql_query(conn, query))
  {
    snprintf(responseBuffer, BUFFER_SIZE - 1, "Error executing SQL query: %s", mysql_error(conn));
    return;
  }

  MYSQL_RES *result = mysql_store_result(conn);
  MYSQL_ROW row;

  memset(responseBuffer, 0, BUFFER_SIZE);
  while ((row = mysql_fetch_row(result)))
  {
    strncat(responseBuffer, row[0], BUFFER_SIZE - strlen(responseBuffer) - 1);
    strncat(responseBuffer, "\n", BUFFER_SIZE - strlen(responseBuffer) - 1);
  }

  mysql_free_result(result);
}

void get_mp3_link_by_index(int index, char *link)
{
  char query[300];
  sprintf(query, "SELECT link FROM mp3Files WHERE id=%d", index);

  if (mysql_query(conn, query))
  {
    snprintf(link, BUFFER_SIZE - 1, "Error executing SQL query: %s", mysql_error(conn));
    return;
  }

  MYSQL_RES *result = mysql_store_result(conn);
  MYSQL_ROW row = mysql_fetch_row(result);

  if (row && row[0])
  {
    strncpy(link, row[0], BUFFER_SIZE - 1);
  }
  else
  {
    strncpy(link, "No link found.", BUFFER_SIZE - 1);
  }

  mysql_free_result(result);
}

void *handle_db_request(void *arg)
{
  int client_socket = *(int *)arg;
  char buffer[BUFFER_SIZE];

  while (1)
  {
    memset(buffer, 0, BUFFER_SIZE);
    ssize_t bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

    buffer[bytes_read] = '\0';

    if (strcmp(buffer, "LIST") == 0)
    {
      char responseBuffer[BUFFER_SIZE] = {0};
      fetch_mp3_files(responseBuffer);
      send(client_socket, responseBuffer, strlen(responseBuffer), 0);
    }
    else if (strncmp(buffer, "GET MP3 LINK", 12) == 0)
    {
      int index;
      if (sscanf(buffer, "GET MP3 LINK %d", &index) == 1)
      {
        char link[BUFFER_SIZE] = {0};
        get_mp3_link_by_index(index, link);
        send(client_socket, link, strlen(link), 0);
      }
    }
  }

  close(client_socket);
  free(arg);
  return NULL;
}

int main()
{
  connect_to_database();

  int server_socket = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in server_address;
  socklen_t addr_len = sizeof(server_address);

  memset(&server_address, 0, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(DB_SERVER_PORT);
  server_address.sin_addr.s_addr = htonl(INADDR_ANY);

  bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address));
  listen(server_socket, 5);

  printf("DB Server started and listening on port %d\n", DB_SERVER_PORT);

  while (1)
  {
    int client_socket = accept(server_socket, (struct sockaddr *)&server_address, &addr_len);
    int *new_sock = malloc(sizeof(int));
    *new_sock = client_socket;

    pthread_t client_thread;
    pthread_create(&client_thread, NULL, handle_db_request, (void *)new_sock);
    pthread_detach(client_thread);
  }

  close(server_socket);
  mysql_close(conn);
  return 0;
}
