#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif
#define _GNU_SOURCE

#include "server.h"

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "command.h"
#include "util.h"

echo_server* make_echo_server(struct sockaddr_in ip_addr, int max_backlog) {
  echo_server* server = malloc(sizeof(echo_server));
  server->listener = open_tcp_socket();
  server->addr = ip_addr;
  server->max_backlog = max_backlog;
  return server;
}

void free_echo_server(echo_server* server) {
  close_tcp_socket(server->listener);
  free(server);
}

void listen_for_connections(echo_server* server) {
  int reuse = 1;
  int listener = server->listener;
  struct sockaddr_in addr = server->addr;
  if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse,
                 sizeof(int)) == -1) {
    error_and_exit("Can't reuse socket");
  }
  int bind_status = bind(listener, (struct sockaddr*)&addr, sizeof(addr));
  if (bind_status == -1) {
    error_and_exit("Can't bind to socket");
  }
  if (listen(listener, server->max_backlog) == -1) {
    error_and_exit("Can't listen");
  }
}

int accept_client(echo_server* server, pthread_mutex_t* mutex, int userID,
                  sqlite3* database) {
  struct sockaddr_storage client_addr;
  unsigned int address_size = sizeof(client_addr);
  int connect_d = accept4(server->listener, (struct sockaddr*)&client_addr,
                          &address_size, SOCK_CLOEXEC);
  if (connect_d == -1) {
    error_and_exit("Can't open secondary socket");
  }
  if (!fork()) {
    echo(connect_d, mutex, userID, database);
    return -1;
  }
  return 0;
}

void echo(int socket_descriptor, pthread_mutex_t* mutex, int userID,
          sqlite3* database) {
  FILE* comm_file = fdopen(socket_descriptor, "r+");
  if (comm_file == NULL) {
    error_and_exit("Can't open file");
  }

  // Send welcome message
  if (fputs(" $$$$$$\\  $$\\      $$\\  $$$$$$\\ \r\n"
            "$$  __$$\\ $$$\\    $$$ |$$  __$$\\\r\n"
            "$$ /  $$ |$$$$\\  $$$$ |$$ /  \\__|\r\n"
            "$$ |  $$ |$$\\$$\\$$ $$ |$$ |$$$$\\\r\n"
            "$$ |  $$ |$$ \\$$$  $$ |$$ |\\_$$ |\r\n"
            "$$ |  $$ |$$ |\\$  /$$ |$$ |  $$ |\r\n"
            " $$$$$$  |$$ | \\_/ $$ |\\$$$$$$  |\r\n"
            " \\______/ \\__|     \\__| \\______/ \r\n",
            comm_file) == -1) {
    error_and_exit("Error sending line!");
  }

  while (!feof(comm_file)) {
    char* buffer = NULL;
    size_t buf_size = 0;
    if (getline(&buffer, &buf_size, comm_file) == -1) {
      error_and_exit("Can't get line");
    }
    string_array* command_tokens = tokenize_line(buffer);
    if (strcasecmp(command_tokens->strings[0], "myInventory") == 0) {
      // Handle myInventory
    } else if (strcasecmp(command_tokens->strings[0], "buy") == 0) {
      order* buy_order = create_order_from_string(command_tokens, userID);
      if (buy(database, buy_order) == -1) {
        if (fputs("Can't create buy order!", comm_file) == EOF) {
          error_and_exit("Couldn't send line");
        }
      }
    } else if (strcasecmp(command_tokens->strings[0], "sell") == 0) {
      order* sell_order = create_order_from_string(command_tokens, userID);
      if (sell(database, sell_order) == -1) {
        if (fputs("Can't create buy order!", comm_file) == EOF) {
          error_and_exit("Couldn't send line");
        }
      }
    } else if (strcasecmp(command_tokens->strings[0], "myOrders") == 0) {
      // Handle "myOrders" command
    } else if (strcasecmp(command_tokens->strings[0], "cancelOrder") == 0) {
      // Handle "cancelOrder" command
    } else if (strcasecmp(command_tokens->strings[0], "view") == 0) {
      // Handle "view" command
    } else {
      // Handle unknown command
    }

    free(buffer);
    free_string_array(command_tokens);
  }
  puts("EXITED");
}
