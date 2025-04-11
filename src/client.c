#include "client.h"

#include <netinet/in.h>  // sockaddr_in
#include <stdio.h>       // getline
#include <stdlib.h>      // free
#include <sys/socket.h>  // connect, sockaddr

#include "util.h"

void try_connect(int client_socket, struct sockaddr_in server_addr) {
  if (connect(client_socket, (struct sockaddr*)&server_addr,
              sizeof(server_addr)) == -1) {
    error_and_exit("Failed to connect!");
  }
}

FILE* get_socket_file(int client_socket) {
  FILE* socket_file = fdopen(client_socket, "w+");
  if (socket_file == NULL) {
    error_and_exit("Couldn't open socket as file stream");
  }
  return socket_file;
}

int echo(FILE* socket_file) {
  char* send_line = NULL;
  size_t send_line_size = 0;
  if (getline(&send_line, &send_line_size, stdin) == -1) {
    return -1;
  }
  // If we can't send the line on the socket, the connection is broken and we
  // have to exit.
  if (fputs(send_line, socket_file) == EOF) {
    free(send_line);
    error_and_exit("Couldn't send line");
  }
  free(send_line);
  char* recv_line = NULL;
  size_t recv_line_size = 0;
  if (getline(&recv_line, &recv_line_size, socket_file) == -1) {
    return -1;
  }
  fputs(recv_line, stdout);
  free(recv_line);
  return 0;
}
