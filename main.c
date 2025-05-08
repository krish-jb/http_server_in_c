#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#define PORT 8080
#define BUFFER_SIZE 1024

bool read_from_file(int sockfd, char buffer[], size_t buffer_size);
bool write_to_file(int sockfd, char resp[], size_t buffer_size);
bool get_sock_info(int sockfd, struct sockaddr_in *client_addr, int *client_addrlen);
void scan_print_info(char buffer[], char version[], char uri[], char method[], struct sockaddr_in *client_addr);

int main() {
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  char buffer[BUFFER_SIZE];
  char resp[] = "HTTP/1.0 200 OK\r\n"
    "Server: webserver-c\r\n"
    "Content-type: text/html\r\n\r\n"
    "<html>Data received!</html>\r\n";

  if (sockfd == -1) {
    perror("webserver (socket)");
    return 1;
  }
  printf("Scoket created successfully.\n");

  struct sockaddr_in host_addr;
  int host_addrlen = sizeof(host_addr);

  struct sockaddr_in client_addr;
  int client_addrlen = sizeof(client_addr);

  host_addr.sin_family = AF_INET;
  host_addr.sin_port = htons(PORT);
  host_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(sockfd, (const struct sockaddr *)&host_addr, host_addrlen) != 0) {
    perror("webserver (bind)");
    return 1;
  }
  printf("Socket successfully bound to address.\n");

  if (listen(sockfd, SOMAXCONN) != 0) {
    perror("webserver (listen)");
    return 1;
  }
  printf("Listenting on port %d...\n", PORT);

  while (1) {
    int newsockfd = accept(sockfd, (struct sockaddr *)&host_addr, (socklen_t *)&host_addrlen);
    if (newsockfd < 0) {
      perror("webserver (accept)");
      return 1;
    }
    printf("Connection accepted.\n");

    if (!get_sock_info(newsockfd, &client_addr, &client_addrlen)) { continue; }

    if (!read_from_file(newsockfd, buffer, BUFFER_SIZE)) { continue; }

    char version[BUFFER_SIZE], uri[BUFFER_SIZE], method[BUFFER_SIZE];
    scan_print_info(buffer, version, uri, method, &client_addr);

    if(!write_to_file(newsockfd, resp, strlen(resp))) { continue; }

    close(newsockfd);
  }

  return 0;
}

bool read_from_file(int sockfd, char buffer[], size_t buffer_size) {
  if (read(sockfd, buffer, BUFFER_SIZE) < 0) {
    perror("werbserver (read)");
    return false;
  }
  return true;
}

bool write_to_file(int sockfd, char resp[], size_t resp_size) {
  if (write(sockfd, resp, resp_size) < 0) {
    perror("webserver (write)");
    return false;
  }
  return true;
}

bool get_sock_info(int sockfd, struct sockaddr_in *client_addr, int *client_addrlen) {
  if (getsockname(sockfd, (struct sockaddr *)client_addr, (socklen_t *)client_addrlen) < 0) {
    perror("webserver (getsockname)");
    return false;
  }
  return true;
}

void scan_print_info(char buffer[], char version[], char uri[], char method[], struct sockaddr_in *client_addr) {
  sscanf(buffer, "%s %s %s", method, uri, version);
  printf("[%s:%u] %s %s %s\n", inet_ntoa(client_addr->sin_addr),
         ntohs(client_addr->sin_port), method, version, uri);
}
