#include "command.h"

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

int open_db(sqlite3** database) {
  *database = open_database();
  if (*database == NULL) {
    return -1;  // Return -1 if database opening fails
  }
  return 0;  // Return 0 on success
}

int init_db(sqlite3* database) {
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

int buy(sqlite3* database, order* ord) { return insert_order(database, ord); }

int sell(sqlite3* database, order* ord) { return insert_order(database, ord); }

void myOrders(sqlite3* database, int userID, order** orderList,
              int* orderCount) {
  // select_user_orders(database, userID, orderList, orderCount);
}

// change to return struct orders??
order* view(int itemID) {}
