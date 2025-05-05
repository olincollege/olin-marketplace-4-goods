#include "command.h"

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // Include for strlen and strcpy

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

user* create_user(int userID, const char* username, const char* password,
                  const char* name, int OMG, int DOGE, int BTC, int ETH) {
  user* new_user = (user*)malloc(sizeof(user));
  if (new_user == NULL) {
    return NULL;  // Return NULL if memory allocation fails
  }

  new_user->userID = userID;
  new_user->username = username;
  new_user->password = password;
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

int free_user(user* user_ptr) {
  if (user_ptr == NULL) {
    return -1;
  }

  if (user_ptr->username != NULL) {
    free(user_ptr->username);
    user_ptr->username = NULL;
  }

  if (user_ptr->password != NULL) {
    free(user_ptr->password);
    user_ptr->password = NULL;
  }

  if (user_ptr->name != NULL) {
    free(user_ptr->name);
    user_ptr->name = NULL;
  }

  free(user_ptr);
  return 0;
}

int buy(sqlite3* database, order* ord) {
  user current_user;
  if (get_user(database, ord->userID, &current_user) != 0) {
    fprintf(stderr, "Error: Failed to retrieve user information.\n");
    return -1;
  }

  double total_cost = ord->quantity * ord->unitPrice;
  if (current_user.OMG < total_cost) {
    fprintf(stderr, "Error: Insufficient funds to place the buy order.\n");
    return -1;
  }

  int result = find_matching_sell(database, ord);
  if (result == -1) {
    return insert_order(database, ord);
  }
  order matched_order;
  if (get_order(database, result, &matched_order) != 0) {
    fprintf(stderr, "Error: Failed to retrieve matching order.\n");
    return -1;
  }

  if (assign_order_timestamp(database, ord) != 0) {
    fprintf(stderr, "Error: Failed to assign timestamp to the order.\n");
    return -1;
  }

  // Archive the matched order
  if (archive_order(database, &matched_order) != 0) {
    fprintf(stderr, "Error: Failed to archive matched order.\n");
    return -1;
  }

  // Archive the buy order
  if (archive_order(database, ord) != 0) {
    fprintf(stderr, "Error: Failed to archive buy order.\n");
    return -1;
  }

  // Update quantities
  int transaction_quantity = (ord->quantity > matched_order.quantity)
                                 ? matched_order.quantity
                                 : ord->quantity;
  double transaction_cost = transaction_quantity * matched_order.unitPrice;

  ord->quantity -= transaction_quantity;
  matched_order.quantity -= transaction_quantity;

  // Update user balances
  current_user.OMG -= transaction_cost;
  if (ord->item == COIN_OMG) {
    current_user.OMG += transaction_quantity;
  } else if (ord->item == COIN_DOGE) {
    current_user.DOGE += transaction_quantity;
  } else if (ord->item == COIN_BTC) {
    current_user.BTC += transaction_quantity;
  } else if (ord->item == COIN_ETH) {
    current_user.ETH += transaction_quantity;
  }

  if (update_user_balance(database, &current_user) != 0) {
    fprintf(stderr, "Error: Failed to update buyer's balance.\n");
    return -1;
  }

  user seller_user;
  if (get_user(database, matched_order.userID, &seller_user) != 0) {
    fprintf(stderr, "Error: Failed to retrieve seller information.\n");
    return -1;
  }

  seller_user.OMG += transaction_cost;
  if (matched_order.item == COIN_OMG) {
    seller_user.OMG -= transaction_quantity;
  } else if (matched_order.item == COIN_DOGE) {
    seller_user.DOGE -= transaction_quantity;
  } else if (matched_order.item == COIN_BTC) {
    seller_user.BTC -= transaction_quantity;
  } else if (matched_order.item == COIN_ETH) {
    seller_user.ETH -= transaction_quantity;
  }

  if (update_user_balance(database, &seller_user) != 0) {
    fprintf(stderr, "Error: Failed to update seller's balance.\n");
    return -1;
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
  user current_user;
  if (get_user(database, ord->userID, &current_user) != 0) {
    fprintf(stderr, "Error: Failed to retrieve user information.\n");
    return -1;
  }

  if (ord->item == COIN_OMG && current_user.OMG < ord->quantity) {
    fprintf(stderr, "Error: Insufficient OMG to place the sell order.\n");
    return -1;
  } else if (ord->item == COIN_DOGE && current_user.DOGE < ord->quantity) {
    fprintf(stderr, "Error: Insufficient DOGE to place the sell order.\n");
    return -1;
  } else if (ord->item == COIN_BTC && current_user.BTC < ord->quantity) {
    fprintf(stderr, "Error: Insufficient BTC to place the sell order.\n");
    return -1;
  } else if (ord->item == COIN_ETH && current_user.ETH < ord->quantity) {
    fprintf(stderr, "Error: Insufficient ETH to place the sell order.\n");
    return -1;
  }

  int result = find_matching_buy(database, ord);
  if (result == -1) {
    return insert_order(database, ord);
  }
  order matched_order;
  if (get_order(database, result, &matched_order) != 0) {
    fprintf(stderr, "Error: Failed to retrieve matching order.\n");
    return -1;
  }

  if (assign_order_timestamp(database, ord) != 0) {
    fprintf(stderr, "Error: Failed to assign timestamp to the order.\n");
    return -1;
  }

  // Archive the matched order
  if (archive_order(database, &matched_order) != 0) {
    fprintf(stderr, "Error: Failed to archive matched order.\n");
    return -1;
  }

  // Archive the sell order
  if (archive_order(database, ord) != 0) {
    fprintf(stderr, "Error: Failed to archive sell order.\n");
    return -1;
  }

  // Update quantities
  int transaction_quantity = (ord->quantity > matched_order.quantity)
                                 ? matched_order.quantity
                                 : ord->quantity;
  double transaction_cost = transaction_quantity * matched_order.unitPrice;

  ord->quantity -= transaction_quantity;
  matched_order.quantity -= transaction_quantity;

  // Update user balances
  if (ord->item == COIN_OMG) {
    current_user.OMG -= transaction_quantity;
  } else if (ord->item == COIN_DOGE) {
    current_user.DOGE -= transaction_quantity;
  } else if (ord->item == COIN_BTC) {
    current_user.BTC -= transaction_quantity;
  } else if (ord->item == COIN_ETH) {
    current_user.ETH -= transaction_quantity;
  }

  current_user.OMG += transaction_cost;

  if (update_user_balance(database, &current_user) != 0) {
    fprintf(stderr, "Error: Failed to update seller's balance.\n");
    return -1;
  }

  user buyer_user;
  if (get_user(database, matched_order.userID, &buyer_user) != 0) {
    fprintf(stderr, "Error: Failed to retrieve buyer information.\n");
    return -1;
  }

  buyer_user.OMG -= transaction_cost;
  if (matched_order.item == COIN_OMG) {
    buyer_user.OMG += transaction_quantity;
  } else if (matched_order.item == COIN_DOGE) {
    buyer_user.DOGE += transaction_quantity;
  } else if (matched_order.item == COIN_BTC) {
    buyer_user.BTC += transaction_quantity;
  } else if (matched_order.item == COIN_ETH) {
    buyer_user.ETH += transaction_quantity;
  }

  if (update_user_balance(database, &buyer_user) != 0) {
    fprintf(stderr, "Error: Failed to update buyer's balance.\n");
    return -1;
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
  int result = get_user_all_orders(database, userID, orderList, orderCount);
  if (result != SQLITE_OK) {
    fprintf(stderr,
            "Error: Failed to retrieve user orders. SQLite error code: %d\n",
            result);
    orderList = NULL;
    *orderCount = 0;
  }
}

int free_order_list(order* orderList, int orderCount) {
  if (orderList == NULL) {
    return -1;  // Return -1 if the order list is NULL
  }

  for (int i = 0; i < orderCount; i++) {
    if (orderList[i].created_at != NULL) {
      free(orderList[i].created_at);  // Free the created_at string
    }
  }

  free(orderList);  // Free the order list array
  return 0;         // Return 0 on successful free
}

void viewItemOrders(sqlite3* database, int itemID, order** buy_orders,
                    int* buy_count, order** sell_orders, int* sell_count) {
  int result = get_item_all_orders(database, itemID, buy_orders, buy_count,
                                   sell_orders, sell_count);
  if (result != SQLITE_OK) {
    fprintf(stderr,
            "Error: Failed to retrieve item orders. SQLite error code: %d\n",
            result);
    return;
  }
}

int get_user_inventory(sqlite3* database, user* usr) {
  return get_user_inventories(database, usr);
}

int get_user_with_username(sqlite3* database, const char* username, user* usr) {
  return get_user_by_username(database, username, usr);
}

int archive_order(sqlite3* database, const order* archived_order) {
  return insert_archive(database, archived_order);
}

void getArchivedOrders(sqlite3* database, int userID, order** orders_out,
                       int* count_out) {
  int result =
      get_user_archived_orders(database, userID, orders_out, count_out);
  if (result != SQLITE_OK) {
    fprintf(
        stderr,
        "Error: Failed to retrieve archived orders. SQLite error code: %d\n",
        result);
    *orders_out = NULL;
    *count_out = 0;
  }
}

int cancelOrder(sqlite3* database, int orderID, int currentUserID) {
  order ord;
  if (get_order(database, orderID, &ord) != 0) {
    fprintf(stderr, "Error: Failed to retrieve order with ID %d.\n", orderID);
    return -1;
  }

  // Verify that the order belongs to the current user
  if (ord.userID != currentUserID) {
    fprintf(stderr, "Error: Unauthorized attempt to cancel order with ID %d.\n",
            orderID);
    return -1;
  }

  user usr;
  if (get_user(database, ord.userID, &usr) != 0) {
    fprintf(stderr, "Error: Failed to retrieve user information.\n");
    return -1;
  }

  if (ord.buyOrSell == 0) {  // Buy order
    double refund_amount = ord.quantity * ord.unitPrice;
    usr.OMG += refund_amount;
  } else {  // Sell order
    if (ord.item == COIN_OMG) {
      usr.OMG += ord.quantity;
    } else if (ord.item == COIN_DOGE) {
      usr.DOGE += ord.quantity;
    } else if (ord.item == COIN_BTC) {
      usr.BTC += ord.quantity;
    } else if (ord.item == COIN_ETH) {
      usr.ETH += ord.quantity;
    }
  }

  if (update_user_balance(database, &usr) != 0) {
    fprintf(stderr, "Error: Failed to update user balance.\n");
    return -1;
  }

  if (delete_order(database, orderID) != 0) {
    fprintf(stderr, "Error: Failed to delete order with ID %d.\n", orderID);
    return -1;
  }

  return 0;
}
