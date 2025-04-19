#include "command.h"

#include <sqlite3.h>

int init_db(sqlite3** database) {
  *database = open_database(database_FILENAME);
  return 0;  // Return 0 on success
}

int close_db(sqlite3* database) {
  close_database(database);
  return 0;  // Return 0 on success
}

int myInventory(int userID, user** cur_user) { return -1; }

// return -1 if unsuccessful
int buy(sqlite3* database, order* ord) { return insert_order(database, ord); }

// return -1 if unsuccessful
int sell(sqlite3* database, order* ord) { return insert_order(database, ord); }

void myOrders(sqlite3* database, int userID, order** orderList,
              int* orderCount) {
  select_user_orders(database, userID, orderList, orderCount);
}

// change to return struct orders??
order* view(int itemID) {}
