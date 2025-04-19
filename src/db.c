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
  // Set busy timeout to 1000 milliseconds (1 second)
  // if the db is locked/busy when accessed, this will make the process retry
  // for 1 sec
  sqlite3_busy_timeout(database, 1000);
  return database;
}

// Closes the database
int close_database(sqlite3* database) {
  if (database) {
    int res = sqlite3_close(database);
    if (res != SQLITE_OK) {
      fprintf(stderr, "Error closing database: %s\n", sqlite3_errstr(res));
      return -1;
    }
  }
  return SQLITE_OK;
}

// Creates the required tables for the database
int create_tables(sqlite3* database) {
  const char* create_tables_sql =
      "CREATE TABLE IF NOT EXISTS users ("
      "userID INTEGER PRIMARY KEY AUTOINCREMENT, "
      "name TEXT NOT NULL, "
      "OMG INTEGER DEFAULT 0, "
      "DOGE INTEGER DEFAULT 0, "
      "BTC INTEGER DEFAULT 0, "
      "ETH INTEGER DEFAULT 0);"

      "CREATE TABLE IF NOT EXISTS orders ("
      "orderID INTEGER PRIMARY KEY AUTOINCREMENT, "
      "item INTEGER NOT NULL, "
      "buyOrSell INTEGER NOT NULL, "
      "quantity INTEGER NOT NULL, "
      "unitPrice REAL NOT NULL, "
      "userID INTEGER NOT NULL, "
      "created_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
      "FOREIGN KEY(userID) REFERENCES users(userID));"

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
  int res = sqlite3_exec(database, create_tables_sql, 0, 0, &errMsg);
  if (res != SQLITE_OK) {
    fprintf(stderr, "Error creating tables: %s\n", errMsg);
    sqlite3_free(errMsg);
    return res;
  }

  return SQLITE_OK;
}

int drop_all_tables(sqlite3* database) {
  char* errMsg = 0;
  const char* sql =
      "PRAGMA foreign_keys = OFF;"  // Temporarily disable FK constraints
      "BEGIN TRANSACTION;"
      "DROP TABLE IF EXISTS users;"
      "DROP TABLE IF EXISTS orders;"
      "DROP TABLE IF EXISTS archives;"
      "COMMIT;"
      "PRAGMA foreign_keys = ON;";

  int res = sqlite3_exec(database, sql, 0, 0, &errMsg);
  if (res != SQLITE_OK) {
    if (errMsg) {
      fprintf(stderr, "Error dropping tables: %s\n", errMsg);
      sqlite3_free(errMsg);
    } else {
      fprintf(stderr, "Error dropping tables: Unknown error\n");
    }
    return SQLITE_ERROR;
  }

  return SQLITE_OK;
}

int insert_order(sqlite3* database, order* new_order) {
  const char* sql =
      "INSERT INTO orders (item, buyOrSell, quantity, unitPrice, userID) "
      "VALUES (?, ?, ?, ?, ?);";

  sqlite3_stmt* stmt = NULL;
  int res = sqlite3_prepare_v2(database, sql, -1, &stmt, NULL);
  if (res != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement: %s\n",
            sqlite3_errmsg(database));
    return res;
  }

  sqlite3_bind_int(stmt, 1, new_order->item);
  sqlite3_bind_int(stmt, 2, new_order->buyOrSell);
  sqlite3_bind_int(stmt, 3, new_order->quantity);
  sqlite3_bind_double(stmt, 4, new_order->unitPrice);
  sqlite3_bind_int(stmt, 5, new_order->userID);

  res = sqlite3_step(stmt);
  if (res != SQLITE_DONE) {
    fprintf(stderr, "Failed to execute statement: %s\n",
            sqlite3_errmsg(database));
    sqlite3_finalize(stmt);
    return res;
  }

  sqlite3_finalize(stmt);
  return SQLITE_OK;
}
