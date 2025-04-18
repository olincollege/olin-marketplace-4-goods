// TODO: discuss if we need structs for inventories and stuff
#include <sqlite3.h>

#include "db.c"

typedef enum { BUY = 0, SELL = 1 } TransactionType;

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
  char* created_at;  // Timestamp for when the order was created
} order;

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
