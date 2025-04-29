#include "command.h"

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // Include for strlen and strcpy

#include "util.h"

int open_db(sqlite3** database) {
  *database = open_database();
  if (*database == NULL) {
    return -1;  // Return -1 if database opening fails
  }
  return 0;  // Return 0 on success
}

int init_db(sqlite3* database) {
  if (drop_all_tables(database) != 0) {
    fprintf(stderr, "Error: Failed to drop all tables.\n");
    if (close_database(database) != 0) {
      fprintf(
          stderr,
          "Error: Failed to close the database after table drop failure.\n");
    }
    return -1;  // Return -1 on failure to drop tables
  }
  if (create_tables(database) != 0) {
    if (close_database(database) != 0) {
      fprintf(stderr,
              "Error: Failed to close the database after table creation "
              "failure.\n");
      return -1;
    };  // Clean up if table creation fails
    return -1;  // Return -1 on failure to create tables
  }

  return 0;  // Return 0 on success
}

int close_db(sqlite3* database) {
  close_database(database);
  return 0;  // Return 0 on success
}

int myInventory(int userID, user** cur_user) { return -1; }

order* create_order_from_string(string_array* params, int userID) {
  order* new_order = (order*)malloc(sizeof(order));
  if (new_order == NULL) {
    return NULL;
  }

  char* endptr;

  // Parse unit price
  new_order->unitPrice = strtod(params->strings[2], &endptr);
  if (endptr == params->strings[2]) {
    printf("Conversion failed for unitPrice\n");
  }

  if (strcasecmp(params->strings[1], "omg") == 0) {
    new_order->item = COIN_OMG;
  } else if (strcasecmp(params->strings[1], "doge") == 0) {
    new_order->item = COIN_DOGE;
  } else if (strcasecmp(params->strings[1], "btc") == 0) {
    new_order->item = COIN_BTC;
  } else if (strcasecmp(params->strings[1], "eth") == 0) {
    new_order->item = COIN_ETH;
  } else {
    printf("Invalid item type: %s\n", params->strings[1]);
    return NULL;
  }

  // Determine buy or sell
  new_order->buyOrSell = (strcasecmp(params->strings[0], "buy") == 0) ? 0 : 1;

  // Parse quantity
  endptr = NULL;
  new_order->quantity = strtol(params->strings[3], &endptr, 10);
  if (endptr == params->strings[3]) {
    printf("Conversion failed for quantity\n");
  }

  // Assign user ID
  new_order->userID = userID;

  return new_order;
}

order* create_order(int item, int buyOrSell, int quantity, double unitPrice,
                    int userID) {
  order* new_order = (order*)malloc(sizeof(order));
  if (new_order == NULL) {
    return NULL;
  }

  new_order->item = item;
  new_order->buyOrSell = buyOrSell;
  new_order->quantity = quantity;
  new_order->unitPrice = unitPrice;
  new_order->userID = userID;

  return new_order;
}

int free_order(order* ord) {
  if (ord == NULL) {
    return -1;  // Return -1 if the order is NULL
  }
  free(ord);
  return 0;  // Return 0 on successful free
}

user* create_user(int userID, const char* name, int OMG, int DOGE, int BTC,
                  int ETH) {
  user* new_user = (user*)malloc(sizeof(user));
  if (new_user == NULL) {
    return NULL;  // Return NULL if memory allocation fails
  }

  new_user->userID = userID;
  new_user->OMG = OMG;
  new_user->DOGE = DOGE;
  new_user->BTC = BTC;
  new_user->ETH = ETH;

  new_user->name = (char*)malloc(strlen(name) + 1);
  if (new_user->name == NULL) {
    free(new_user);
    return NULL;  // Return NULL if memory allocation for name fails
  }
  strncpy(new_user->name, name, strlen(name) + 1);

  return new_user;  // Return the created user
}

int free_user(user* usr) {
  if (usr == NULL) {
    return -1;  // Return -1 if the user is NULL
  }

  if (usr->name != NULL) {
    free(usr->name);  // Free the name string
  }
  free(usr);  // Free the user struct
  return 0;   // Return 0 on successful free
}

int buy(sqlite3* database, order* ord) {
  int result = find_matching_sell(database, ord);
  if (result == -1) {
    return insert_order(database, ord);
  }
  order matched_order;
  if (get_order(database, result, &matched_order) != 0) {
    fprintf(stderr, "Error: Failed to retrieve matching order.\n");
    return -1;
  }

  // Update quantities
  if (ord->quantity > matched_order.quantity) {
    ord->quantity -= matched_order.quantity;
    matched_order.quantity = 0;
  } else {
    matched_order.quantity -= ord->quantity;
    ord->quantity = 0;
  }

  // Delete or update the matched order
  if (matched_order.quantity == 0) {
    if (delete_order(database, matched_order.orderID) != 0) {
      fprintf(stderr, "Error: Failed to delete matched order.\n");
      return -1;
    }
  } else {
    if (update_order(database, &matched_order) != 0) {
      fprintf(stderr, "Error: Failed to update matched order.\n");
      return -1;
    }
  }

  // Insert the remaining order if not fully matched
  if (ord->quantity > 0) {
    if (insert_order(database, ord) != 0) {
      fprintf(stderr, "Error: Failed to insert remaining order.\n");
      return -1;
    }
  }

  return 0;
}

int sell(sqlite3* database, order* ord) {
  int result = find_matching_buy(database, ord);
  if (result == -1) {
    return insert_order(database, ord);
  }
  order matched_order;
  if (get_order(database, result, &matched_order) != 0) {
    fprintf(stderr, "Error: Failed to retrieve matching order.\n");
    return -1;
  }

  // Update quantities
  if (ord->quantity > matched_order.quantity) {
    ord->quantity -= matched_order.quantity;
    matched_order.quantity = 0;
  } else {
    matched_order.quantity -= ord->quantity;
    ord->quantity = 0;
  }

  // Delete or update the matched order
  if (matched_order.quantity == 0) {
    if (delete_order(database, matched_order.orderID) != 0) {
      fprintf(stderr, "Error: Failed to delete matched order.\n");
      return -1;
    }
  } else {
    if (update_order(database, &matched_order) != 0) {
      fprintf(stderr, "Error: Failed to update matched order.\n");
      return -1;
    }
  }

  // Insert the remaining order if not fully matched
  if (ord->quantity > 0) {
    if (insert_order(database, ord) != 0) {
      fprintf(stderr, "Error: Failed to insert remaining order.\n");
      return -1;
    }
  }

  return 0;
}

void myOrders(sqlite3* database, int userID, order** orderList,
              int* orderCount) {
  // select_user_orders(database, userID, orderList, orderCount);
}

// change to return struct orders??
order* view(int itemID) {}
