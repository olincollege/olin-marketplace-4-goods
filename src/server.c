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
#include "db.h"
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

int accept_client(echo_server* server, sqlite3* database) {
  struct sockaddr_storage client_addr;
  unsigned int address_size = sizeof(client_addr);
  int connect_d = accept4(server->listener, (struct sockaddr*)&client_addr,
                          &address_size, SOCK_CLOEXEC);
  if (connect_d == -1) {
    error_and_exit("Can't open secondary socket");
  }
  if (!fork()) {
    FILE* comm_file = fdopen(connect_d, "r+");
    int userID = -1;
    int status = 0;
    if (comm_file == NULL) {
      error_and_exit("Can't open file");
    }

    // Either register or log in
    while (userID == -1 || userID == 0 || status != 1) {
      if (fputs(
              "Welcome to OMG. \"r\" to register, and \"u\" for existing users "
              "\r\n",
              comm_file) == EOF) {
        error_and_exit("Couldn't send prompt");
      }
      char* input = NULL;
      size_t input_size = 0;
      if (getline(&input, &input_size, comm_file) == -1) {
        error_and_exit("Couldn't read input");
      }
      // Check the first character of the input
      if (input[0] == 'r') {
        // Handle 'r' case
        userID = register_user(comm_file, database);
      } else if (input[0] == 'u') {
        // Handle 'u' case
        userID = authenticate(comm_file, database);
        status = 1;
      } else {
        // Handle invalid input
        if (fputs("Invalid option. Please enter 'r' or 'u'.\r\n", comm_file) ==
            EOF) {
          error_and_exit("Couldn't send error message");
        }
        (void)fflush(comm_file);
      }
      free(input);
    }
    echo(comm_file, userID, database);
    return -1;
  }
  return 0;
}

int register_user(FILE* comm_file, sqlite3* database) {
  int userID = 0;
  if (fputs("Please enter a username: \r\n", comm_file) == EOF) {
    error_and_exit("Couldn't send prompt");
  }

  char* username = NULL;
  size_t username_size = 0;
  if (getline(&username, &username_size, comm_file) == -1) {
    error_and_exit("Couldn't read username");
  }

  // Remove trailing newline character if present
  size_t len = strlen(username);
  if (len > 0 && username[len - 1] == '\n') {
    username[len - 2] = '\0';
  }

  if (fputs("Please enter your name (for display purposes): \r\n", comm_file) ==
      EOF) {
    error_and_exit("Couldn't send prompt");
  }

  char* name = NULL;
  size_t name_size = 0;
  if (getline(&name, &name_size, comm_file) == -1) {
    error_and_exit("Couldn't read name");
  }

  // Remove trailing newline character if present
  len = strlen(name);
  if (len > 0 && name[len - 1] == '\n') {
    name[len - 2] = '\0';
  }

  if (fputs("Please enter a password: \r\n", comm_file) == EOF) {
    error_and_exit("Couldn't send prompt");
  }

  char* password = NULL;
  size_t password_size = 0;
  if (getline(&password, &password_size, comm_file) == -1) {
    error_and_exit("Couldn't read password");
  }

  // Remove trailing newline character if present
  len = strlen(password);
  if (len > 0 && password[len - 1] == '\n') {
    password[len - 2] = '\0';
  }

  // Register the user in the database
  user new_user = {.username = username,
                   .password = password,
                   .name = name,
                   .BTC = DEFAULT_BTC,
                   .DOGE = DEFAULT_DOGE,
                   .ETH = DEFAULT_ETH,
                   .OMG = DEFAULT_OMG};

  if (insert_user(database, &new_user, &userID) != SQLITE_OK) {
    puts("Error inserting user!");
    return -1;
  }

  if (fputs("Registration successful! You can now log in.\r\n", comm_file) ==
      EOF) {
    error_and_exit("Couldn't send success message");
  }

  free(username);
  free(name);
  free(password);
  return userID;
}

int authenticate(FILE* comm_file, sqlite3* database) {
  int authenticated = 0;
  while (authenticated != 1) {
    dump_database(database);

    // Write "username: " to the socket
    if (fputs("Username: \r\n", comm_file) == EOF) {
      error_and_exit("Couldn't send prompt");
    }

    // Read the input into a char*
    char* username = NULL;
    size_t username_size = 0;
    if (getline(&username, &username_size, comm_file) == -1) {
      error_and_exit("Couldn't read username");
    }

    // Remove trailing newline character if present
    size_t len = strlen(username);
    if (len > 0 && username[len - 1] == '\n') {
      username[len - 2] = '\0';
    }

    user auth_user;
    if (get_user_with_username(database, username, &auth_user) != SQLITE_OK) {
      puts("Name is wrong");
      if (fputs("Failed to authenticate username!\r\n\n", comm_file) == EOF) {
        error_and_exit("Couldn't send error message");
      }
      (void)fflush(comm_file);
      free(username);
      continue;
    }

    // Write "Password: " to the socket
    if (fputs("Password: \r\n", comm_file) == EOF) {
      error_and_exit("Couldn't send prompt");
    }

    // Read the input into a char*
    char* password = NULL;
    size_t password_size = 0;
    if (getline(&password, &password_size, comm_file) == -1) {
      error_and_exit("Couldn't read password");
    }

    // Remove trailing newline character if present
    len = strlen(password);
    if (len > 0 && password[len - 1] == '\n') {
      password[len - 2] = '\0';
    }

    if (strcmp(password, auth_user.password) == 0) {
      (void)fprintf(comm_file, "Welcome back, %s\r\n", auth_user.name);
      (void)fflush(comm_file);
      authenticated = 1;
      free(username);
      free(password);
      return auth_user.userID;
    }

    if (fputs("Password incorrect\r\n\n", comm_file) == EOF) {
      error_and_exit("Couldn't send error message");
    }
    (void)fflush(comm_file);

    // Free allocated memory for username and password
    free(username);
    free(password);
  }
}

void echo(FILE* comm_file, int userID, sqlite3* database) {
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
    dump_database(database);
    char* buffer = NULL;
    char* send_line = NULL;
    size_t buf_size = 0;
    if (getline(&buffer, &buf_size, comm_file) == -1) {
      error_and_exit("Can't get line");
    }

    // Process the command
    string_array* command_tokens = tokenize_line(buffer);
    if (strcasecmp(command_tokens->strings[0], "myInventory") == 0) {
      // Handle myInventory
      user current_user = {.userID = userID};
      if (get_user_inventories(database, &current_user) != SQLITE_OK) {
        if (fputs("Error retrieving inventory!\r\n", comm_file) == EOF) {
          error_and_exit("Couldn't send error message");
        }
        (void)fflush(comm_file);
        continue;
      }

      (void)fprintf(comm_file, "Your current inventory:\r\n");
      (void)fprintf(comm_file, "OMG: %d\r\n", current_user.OMG);
      (void)fprintf(comm_file, "DOGE: %d\r\n", current_user.DOGE);
      (void)fprintf(comm_file, "ETH: %d\r\n", current_user.ETH);
      (void)fprintf(comm_file, "BTC: %d\r\n", current_user.BTC);
      (void)fflush(comm_file);

    } else if (strcasecmp(command_tokens->strings[0], "buy") == 0) {
      // Handle buy
      order* buy_order = create_order_from_string(command_tokens, userID);
      if (buy(database, buy_order) == -1) {
        if (fputs("Can't create buy order!\r\n", comm_file) == EOF) {
          error_and_exit("Couldn't send line");
        }
      } else {
        if (fputs("Successfully created buy order!\r\n", comm_file) == EOF) {
          error_and_exit("Couldn't send success message");
        }
        (void)fflush(comm_file);
      }

    } else if (strcasecmp(command_tokens->strings[0], "sell") == 0) {
      order* sell_order = create_order_from_string(command_tokens, userID);
      if (sell(database, sell_order) == -1) {
        if (fputs("Can't create sell order!\r\n", comm_file) == EOF) {
          error_and_exit("Couldn't send line");
        }
      } else {
        if (fputs("Successfully created sell order!\r\n", comm_file) == EOF) {
          error_and_exit("Couldn't send success message");
        }
        (void)fflush(comm_file);
      }

    } else if (strcasecmp(command_tokens->strings[0], "myOrders") == 0) {
      // Handle "myOrders" command
      order* order_list = NULL;
      int order_count = 0;
      myOrders(database, userID, &order_list, &order_count);
      if (order_list != NULL) {
        for (int i = 0; i < order_count; i++) {
          const char* item;
          if (order_list[i].item == COIN_DOGE) {
            item = "DOGE";
          } else if (order_list[i].item == COIN_BTC) {
            item = "BTC";
          } else if (order_list[i].item == COIN_ETH) {
            item = "ETH";
          } else if (order_list[i].item == COIN_OMG) {
            item = "OMG";
          } else {
            item = "UNKNOWN";
          }
          if (fprintf(comm_file,
                      "Order %d: Type: %s, Item: %s, Amount: %d, Price: "
                      "%.2f, ID: %d\r\n",
                      i + 1, order_list[i].buyOrSell == 0 ? "BUY" : "SELL",
                      item, order_list[i].quantity, order_list[i].unitPrice,
                      order_list[i].orderID) == -1) {
            puts("Error sending order!");
          }
        }
        (void)fflush(comm_file);
        free(order_list);
      } else {
        if (fputs("No orders found.\r\n", comm_file) == EOF) {
          error_and_exit("Couldn't send message");
        }
        (void)fflush(comm_file);
      }

    } else if (strcasecmp(command_tokens->strings[0], "cancelOrder") == 0) {
      // Handle "cancelOrder" command
      if (command_tokens->size != 2) {
        if (fputs("Invalid command syntax! Usage: cancelOrder <orderID>\r\n",
                  comm_file) == EOF) {
          error_and_exit("Couldn't send error message");
        }
        (void)fflush(comm_file);
        continue;
      }
      char* endptr = NULL;
      int orderID = strtol(command_tokens->strings[1], &endptr, 10);
      if (*endptr != '\0') {
        if (fputs("Invalid order ID format!\r\n", comm_file) == EOF) {
          error_and_exit("Couldn't send error message");
        }
        (void)fflush(comm_file);
        continue;
      }

      if (cancelOrder(database, orderID, userID) != 0) {
        if (fputs("Failed to cancel order!\r\n", comm_file) == EOF) {
          error_and_exit("Couldn't send error message");
        }
        (void)fflush(comm_file);
      } else {
        if (fputs("Successfully cancelled order!\r\n", comm_file) == EOF) {
          error_and_exit("Couldn't send success message");
        }
        (void)fflush(comm_file);
      }

    } else if (strcasecmp(command_tokens->strings[0], "view") == 0) {
      // Handle "view" command
      int item = -1;

      if (strcasecmp(command_tokens->strings[1], "omg") == 0) {
        item = COIN_OMG;
      } else if (strcasecmp(command_tokens->strings[1], "doge") == 0) {
        item = COIN_DOGE;
      } else if (strcasecmp(command_tokens->strings[1], "btc") == 0) {
        item = COIN_BTC;
      } else if (strcasecmp(command_tokens->strings[1], "eth") == 0) {
        item = COIN_ETH;
      } else {
        if (fputs("Invalid item type\r\n", comm_file) == EOF) {
          error_and_exit("Couldn't send error message");
        }
        (void)fflush(comm_file);
      }

      order* buy_orders = NULL;
      int buy_count = 0;
      order* sell_orders = NULL;
      int sell_count = 0;

      viewItemOrders(database, item, &buy_orders, &buy_count, &sell_orders,
                     &sell_count);

      // Print header and separator
      if (fprintf(comm_file,
                  "-------------------------------------------\r\n") < 0) {
        error_and_exit("Couldn't send header");
      }
      if (fprintf(comm_file, "%-30s | %s\r\n", "Buy Orders", "Sell Orders") <
          0) {
        error_and_exit("Couldn't send header");
      }

      // Determine maximum number of rows needed
      int max_rows = (buy_count > sell_count) ? buy_count : sell_count;

      // Print each row
      for (int i = 0; i < max_rows; i++) {
        // Print buy order info if available
        if (i < buy_count) {
          if (fprintf(comm_file, "%-30s",
                      fprintf_to_string("price: %.2f, quantity: %d",
                                        buy_orders[i].unitPrice,
                                        buy_orders[i].quantity)) < 0) {
            error_and_exit("Couldn't send buy order info");
          }
        } else {
          // Print empty space if no buy order for this row
          if (fprintf(comm_file, "%-30s", "") < 0) {
            error_and_exit("Couldn't send spacing");
          }
        }

        // Print separator
        if (fprintf(comm_file, " | ") < 0) {
          error_and_exit("Couldn't send separator");
        }

        // Print sell order info if available
        if (i < sell_count) {
          if (fprintf(comm_file, "price: %.2f, quantity: %d",
                      sell_orders[i].unitPrice, sell_orders[i].quantity) < 0) {
            error_and_exit("Couldn't send sell order info");
          }
        }

        // End the row
        if (fprintf(comm_file, "\r\n") < 0) {
          error_and_exit("Couldn't send newline");
        }
      }

    } else {
      // Handle unknown command
      if (fputs("Invalid syntax!\r\n", comm_file) == EOF) {
        error_and_exit("Couldn't send success message");
      }
      (void)fflush(comm_file);
    }

    free(buffer);
    free_string_array(command_tokens);
  }
  puts("EXITED");
}
