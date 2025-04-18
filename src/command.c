#pragma once
// TODO: discuss if we need structs for inventories and stuff
#include <sqlite3.h>

#include "db.c"

typedef enum { BUY = 0, SELL = 1 } TransactionType;

typedef enum {
  COIN_OMG = 0,
  COIN_DOGE = 1,
  COIN_BTC = 2,
  COIN_ETH = 3
} CoinType;

typedef struct {
  int userID;
  char* name;
  int OMG;
  int DOGE;
  int BTC;
  int ETH;
} user;

typedef struct {
  int orderID;
  int item;
  int buyOrSell;  // 0 for buy, 1 for sell
  int quantity;
  int userID;
  double unitPrice;
  char* created_at;  // Timestamp for when the order was created
} order;

// Default database file name
#define DB_FILENAME "omg.db"

int initDb(sqlite3** db) {
  *db = open_database(DB_FILENAME);
  if (*db == NULL) {
    return -1;  // Return -1 if opening the database fails
  }
  return 0;  // Return 0 on success
}

int closeDb(sqlite3* db) {
  if (db == NULL) {
    fprintf(stderr, "Error: Database pointer is NULL.\n");
    return -1;  // Return -1 if the database pointer is NULL
  }
  if (close_database(db) != 0) {
    fprintf(stderr, "Error: Failed to close the database.\n");
    return -1;  // Return -1 if closing the database fails
  }
  return 0;  // Return 0 on success
}

// change to return struct item??
int* myInventory() {}

// return -1 if unsuccessful
int buy(sqlite3* db, order* ord) { return insert_order(db, ord); }

// return -1 if unsuccessful
int sell(sqlite3* db, order* ord) { return insert_order(db, ord); }

// change to return struct orders??
void myOrders(order* orderList, int* orderCount) {
  // TODO: store stuff in orderList as they are allocated outside in the
  // server.c
}

// change to return struct orders??
order* view(int itemID) {}
