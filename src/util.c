#include "util.h"

#include <ctype.h>   // isspace
#include <stdarg.h>  // va_list, va_start, va_end
#include <stdint.h>  // uint16_t
#include <stdio.h>   // perror
#include <stdlib.h>  // exit, EXIT_FAILURE
#include <unistd.h>  // close

#include "db.h"

const uint16_t PORT = 4242;
const size_t INITIAL_TOKENS_CAPACITY = 4;

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

string_array* tokenize_line(const char* line) {
  string_array* tokens = make_string_array(INITIAL_TOKENS_CAPACITY);
  const char* token_start = line;
  size_t token_size = 0;
  for (const char* current_pos = line; *current_pos != '\0'; ++current_pos) {
    if (isspace((int)*current_pos)) {
      // If we are switching from non-whitespace to whitespace, we've reached
      // the end of a token, so add what we currently have.
      if (token_size) {
        add_string(tokens, token_start, token_size);
        token_size = 0;
      }
    } else {
      // If we are switching from whitespace to non-whitespace, we've reached
      // the start of a token, so move the token start position.
      if (token_size == 0) {
        token_start = current_pos;
      }
      ++token_size;
    }
  }
  return tokens;
}

// Helper function to format a string and return it
char* fprintf_to_string(const char* format, ...) {
  va_list args;
  char* result = NULL;

  va_start(args, format);
  vasprintf(&result, format, args);
  va_end(args);

  return result;
}

const char* coin_type_to_string(int coin_type) {
  switch (coin_type) {
    case COIN_DOGE:
      return "DOGE";
    case COIN_BTC:
      return "BTC";
    case COIN_ETH:
      return "ETH";
    case COIN_OMG:
      return "OMG";
    default:
      return "UNKNOWN";
  }
}
