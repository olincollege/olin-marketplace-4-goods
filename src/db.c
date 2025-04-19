#include "db.h"

#include <stdio.h>
#include <stdlib.h>

// Opens a database for use
sqlite3* open_database(void) {
  sqlite3* database = NULL;
  if (sqlite3_open(FILENAME, &database) != SQLITE_OK) {
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(database));
    return NULL;
  }
  return database;
}

// Closes the database
int close_database(sqlite3* database) {
  if (database) {
    int rc = sqlite3_close(database);
    if (rc != SQLITE_OK) {
      fprintf(stderr, "Error closing database: %s\n", sqlite3_errstr(rc));
      return -1;
    }
  }
  return SQLITE_OK;
}

// Creates the required tables for the database
int create_tables(sqlite3* database) {
  const char* users_table_sql =
      "CREATE TABLE IF NOT EXISTS users ("
      "userID INTEGER PRIMARY KEY AUTOINCREMENT, "
      "name TEXT NOT NULL, "
      "OMG INTEGER DEFAULT 0, "
      "DOGE INTEGER DEFAULT 0, "
      "BTC INTEGER DEFAULT 0, "
      "ETH INTEGER DEFAULT 0);";

  const char* orders_table_sql =
      "CREATE TABLE IF NOT EXISTS orders ("
      "orderID INTEGER PRIMARY KEY AUTOINCREMENT, "
      "item INTEGER NOT NULL, "
      "buyOrSell INTEGER NOT NULL, "
      "quantity INTEGER NOT NULL, "
      "unitPrice REAL NOT NULL, "
      "userID INTEGER NOT NULL, "
      "created_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
      "FOREIGN KEY(userID) REFERENCES users(userID));";

  const char* archives_table_sql =
      "CREATE TABLE IF NOT EXISTS archives ("
      "orderID INTEGER PRIMARY KEY AUTOINCREMENT, "
      "item INTEGER NOT NULL, "
      "buyOrSell INTEGER NOT NULL, "
      "quantity INTEGER NOT NULL, "
      "unitPrice REAL NOT NULL, "
      "userID INTEGER NOT NULL, "
      "created_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
      "FOREIGN KEY(userID) REFERENCES users(userID));";

  char* errMsg = 0;
  int rc;

  rc = sqlite3_exec(database, users_table_sql, 0, 0, &errMsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Error creating users table: %s\n", errMsg);
    sqlite3_free(errMsg);
    return rc;
  }

  rc = sqlite3_exec(database, orders_table_sql, 0, 0, &errMsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Error creating orders table: %s\n", errMsg);
    sqlite3_free(errMsg);
    return rc;
  }

  rc = sqlite3_exec(database, archives_table_sql, 0, 0, &errMsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Error creating archives table: %s\n", errMsg);
    sqlite3_free(errMsg);
    return rc;
  }

  return SQLITE_OK;
}

// Insert a user records into the inventory
int insert_item(sqlite3* database, int id, int omg, int coin1, int coin2,
                int coin3) {
  char sql[256];
  snprintf(sql, sizeof(sql),
           "INSERT INTO inventory (id, omg, coin1, coin2, coin3) VALUES (%d, "
           "%d, %d, %d, %d);",
           id, omg, coin1, coin2, coin3);
  return sqlite3_exec(database, sql, 0, 0, 0);
}

// Select callback
int select_callback(void* data, int argc, char** argv, char** col) {
  for (int i = 0; i < argc; i++) {
    printf("%s = %s\n", col[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}

// Select all inventory
int select_all(sqlite3* database) {
  const char* sql = "SELECT * FROM inventory;";
  return sqlite3_exec(database, sql, select_callback, 0, 0);
}

// Update inventory
int update_item(sqlite3* database, int id, int omg, int coin1, int coin2,
                int coin3) {
  char sql[256];
  snprintf(sql, sizeof(sql),
           "UPDATE inventory SET omg = %d, coin1 = %d, coin2 = %d, coin3 = %d "
           "WHERE id = %d;",
           omg, coin1, coin2, coin3, id);
  return sqlite3_exec(database, sql, 0, 0, 0);
}

// Delete user inventory
int delete_item(sqlite3* database, int id) {
  char sql[128];
  snprintf(sql, sizeof(sql), "DELETE FROM inventory WHERE id = %d;", id);
  return sqlite3_exec(database, sql, 0, 0, 0);
}

int insert_order(sqlite3* database, order* new_order) {
  char sql[256];
  snprintf(sql, sizeof(sql),
           "INSERT INTO orders (item, buyOrSell, quantity, unitPrice, userID) "
           "VALUES (%d, %d, %d, %.2f, %d);",
           new_order->item, new_order->buyOrSell, new_order->quantity,
           new_order->unitPrice, new_order->userID);
  return sqlite3_exec(database, sql, 0, 0, 0);
}

// Select orders for a specific user
int select_user_orders(sqlite3* database, int userID, order* orderList,
                       int* orderCount) {}
