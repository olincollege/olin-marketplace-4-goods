#include "util.h"

#include <stdint.h>  // uint16_t
#include <stdio.h>   // perror
#include <stdlib.h>  // exit, EXIT_FAILURE
#include <unistd.h>  // close

const uint16_t PORT = 4242;

void error_and_exit(const char* error_msg) {
  perror(error_msg);
  // NOLINTNEXTLINE(concurrency-mt-unsafe)
  exit(EXIT_FAILURE);
}

int open_tcp_socket(void) {
  int socket_no = socket(PF_INET, SOCK_STREAM, 0);
  if (socket_no == -1) {
    error_and_exit("Can't open socket!");
  }
  return socket_no;
}

void close_tcp_socket(int socket_) {
  if (close(socket_) == -1) {
    error_and_exit("Can't close socket!");
  }
}

struct sockaddr_in socket_address(in_addr_t addr, in_port_t port) {
  struct sockaddr_in name = {.sin_family = PF_INET,
                             .sin_port = (in_port_t)htons(port),
                             .sin_addr.s_addr = htonl(addr)};
  return name;
}
