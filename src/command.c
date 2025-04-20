#include "command.h"

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

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

order* create_order(string_array* params, int userID) {
  order* new_order = (order*)malloc(sizeof(order));
  if (new_order == NULL) {
    return NULL;
  }
  char* endptr;
  // new_order->item = params->strings[1];
  new_order->unitPrice = strtod(params->strings[2], &endptr);
  if (endptr == params->strings[2]) {
    printf("Conversion failed\n");
  }
  new_order->buyOrSell = (strcasecmp(params->strings[0], "buy") == 0) ? 0 : 1;
  endptr = NULL;
  new_order->quantity = strtol(params->strings[3], &endptr, 10);
  if (endptr == params->strings[3]) {
    printf("Conversion failed\n");
  }
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

// return -1 if unsuccessful
int buy(sqlite3* database, order* ord) { return insert_order(database, ord); }

// return -1 if unsuccessful
int sell(sqlite3* database, order* ord) { return insert_order(database, ord); }

void myOrders(sqlite3* database, int userID, order** orderList,
              int* orderCount) {
  // select_user_orders(database, userID, orderList, orderCount);
}

// change to return struct orders??
order* view(int itemID) {}
