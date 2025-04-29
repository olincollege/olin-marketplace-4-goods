#include "db.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  sqlite3_busy_timeout(database, BUSY_TIMEOUT);
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

int insert_user(sqlite3* database, user* new_user) {
  const char* sql =
      "INSERT INTO users (name, OMG, DOGE, BTC, ETH) "
      "VALUES (?, ?, ?, ?, ?);";

  sqlite3_stmt* stmt = NULL;
  int res = sqlite3_prepare_v2(database, sql, -1, &stmt, NULL);
  if (res != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement: %s\n",
            sqlite3_errmsg(database));
    return res;
  }

  sqlite3_bind_text(stmt, 1, new_user->name, -1, SQLITE_STATIC);
  sqlite3_bind_int(stmt, 2, new_user->OMG);
  sqlite3_bind_int(stmt, 3, new_user->DOGE);
  sqlite3_bind_int(stmt, 4, new_user->BTC);
  sqlite3_bind_int(stmt, 5, new_user->ETH);

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

void dump_database(sqlite3* database) {
  const char* queries[] = {"SELECT * FROM users;", "SELECT * FROM orders;",
                           "SELECT * FROM archives;"};
  const char* table_names[] = {"Users", "Orders", "Archives"};

  for (int i = 0; i < 3; i++) {
    printf("Table: %s\n", table_names[i]);
    sqlite3_stmt* stmt = NULL;
    int res = sqlite3_prepare_v2(database, queries[i], -1, &stmt, NULL);
    if (res != SQLITE_OK) {
      fprintf(stderr, "Failed to prepare statement for table %s: %s\n",
              table_names[i], sqlite3_errmsg(database));
      continue;
    }

    int column_count = sqlite3_column_count(stmt);
    for (int col = 0; col < column_count; col++) {
      printf("%s\t", sqlite3_column_name(stmt, col));
    }
    printf("\n");

    while ((res = sqlite3_step(stmt)) == SQLITE_ROW) {
      for (int col = 0; col < column_count; col++) {
        switch (sqlite3_column_type(stmt, col)) {
          case SQLITE_INTEGER:
            printf("%d\t", sqlite3_column_int(stmt, col));
            break;
          case SQLITE_FLOAT:
            printf("%f\t", sqlite3_column_double(stmt, col));
            break;
          case SQLITE_TEXT:
            printf("%s\t", sqlite3_column_text(stmt, col));
            break;
          case SQLITE_NULL:
            printf("NULL\t");
            break;
          default:
            printf("?\t");
        }
      }
      printf("\n");
    }

    if (res != SQLITE_DONE) {
      fprintf(stderr, "Error stepping through table %s: %s\n", table_names[i],
              sqlite3_errmsg(database));
    }

    sqlite3_finalize(stmt);
    printf("\n");
  }
}

int delete_order(sqlite3* database, int orderID) {
  const char* sql = "DELETE FROM orders WHERE orderID = ?;";
  sqlite3_stmt* stmt = NULL;

  int res = sqlite3_prepare_v2(database, sql, -1, &stmt, NULL);
  if (res != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare the delete statement: %s\n",
            sqlite3_errmsg(database));
    return res;
  }

  sqlite3_bind_int(stmt, 1, orderID);

  res = sqlite3_step(stmt);
  if (res != SQLITE_DONE) {
    fprintf(stderr, "Failed to execute on the delete statement: %s\n",
            sqlite3_errmsg(database));
    sqlite3_finalize(stmt);
    return res;
  }

  sqlite3_finalize(stmt);
  return SQLITE_OK;
}

int get_user(sqlite3* database, int userID, user* user_out) {
  const char* sql =
      "SELECT userID, name, OMG, DOGE, BTC, ETH FROM users WHERE userID = ?;";
  sqlite3_stmt* stmt = NULL;

  int res = sqlite3_prepare_v2(database, sql, -1, &stmt, NULL);
  if (res != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare the get_user statement: %s\n",
            sqlite3_errmsg(database));
    return res;
  }

  sqlite3_bind_int(stmt, 1, userID);

  res = sqlite3_step(stmt);
  if (res == SQLITE_ROW) {
    user_out->userID = sqlite3_column_int(stmt, 0);
    const unsigned char* name = sqlite3_column_text(stmt, 1);
    user_out->name = strdup((const char*)name);
    user_out->OMG = sqlite3_column_int(stmt, 2);
    user_out->DOGE = sqlite3_column_int(stmt, 3);
    user_out->BTC = sqlite3_column_int(stmt, 4);
    user_out->ETH = sqlite3_column_int(stmt, 5);
    sqlite3_finalize(stmt);
    return SQLITE_OK;
  }

  fprintf(stderr, "User with ID %d not found.\n", userID);
  sqlite3_finalize(stmt);
  return SQLITE_NOTFOUND;
}

int get_order(sqlite3* database, int orderID, order* order_out) {
  const char* sql =
      "SELECT orderID, item, buyOrSell, quantity, unitPrice, userID, "
      "created_at "
      "FROM orders WHERE orderID = ?;";
  sqlite3_stmt* stmt = NULL;

  int res = sqlite3_prepare_v2(database, sql, -1, &stmt, NULL);
  if (res != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare the get_order statement: %s\n",
            sqlite3_errmsg(database));
    return res;
  }

  sqlite3_bind_int(stmt, 1, orderID);

  res = sqlite3_step(stmt);
  if (res == SQLITE_ROW) {
    order_out->orderID = sqlite3_column_int(stmt, 0);
    order_out->item = sqlite3_column_int(stmt, 1);
    order_out->buyOrSell = sqlite3_column_int(stmt, 2);
    order_out->quantity = sqlite3_column_int(stmt, 3);
    order_out->unitPrice = sqlite3_column_double(stmt, 4);
    order_out->userID = sqlite3_column_int(stmt, 5);
    const unsigned char* created_at = sqlite3_column_text(stmt, 6);
    order_out->created_at = strdup((const char*)created_at);
    sqlite3_finalize(stmt);
    return SQLITE_OK;
  }

  fprintf(stderr, "Order with ID %d not found.\n", orderID);
  sqlite3_finalize(stmt);
  return SQLITE_NOTFOUND;
}

int find_matching_buy(sqlite3* database, order* search_order) {
  const char* sql =
      "SELECT orderID, item, buyOrSell, quantity, unitPrice, userID, "
      "created_at "
      "FROM orders "
      "WHERE item = ? AND buyOrSell = 0 AND unitPrice >= ? AND userID != ? "
      "ORDER BY created_at ASC LIMIT 1;";
  sqlite3_stmt* stmt = NULL;

  int res = sqlite3_prepare_v2(database, sql, -1, &stmt, NULL);
  if (res != SQLITE_OK) {
    fprintf(stderr,
            "Failed to prepare the find_matching_higher statement: %s\n",
            sqlite3_errmsg(database));
    return res;
  }

  sqlite3_bind_int(stmt, 1, search_order->item);
  sqlite3_bind_double(stmt, 2, search_order->unitPrice);
  sqlite3_bind_int(stmt, 3, search_order->userID);

  res = sqlite3_step(stmt);
  if (res == SQLITE_ROW) {
    int orderID = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return orderID;
  }

  fprintf(stderr, "No matching order found with higher price.\n");
  sqlite3_finalize(stmt);
  return -1;  // Return -1 if no matching order is found
}

int find_matching_sell(sqlite3* database, order* search_order) {
  const char* sql =
      "SELECT orderID, item, buyOrSell, quantity, unitPrice, userID, "
      "created_at "
      "FROM orders "
      "WHERE item = ? AND buyOrSell = 1 AND unitPrice <= ? AND userID != ? "
      "ORDER BY created_at ASC LIMIT 1;";
  sqlite3_stmt* stmt = NULL;

  int res = sqlite3_prepare_v2(database, sql, -1, &stmt, NULL);
  if (res != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare the find_matching_sell statement: %s\n",
            sqlite3_errmsg(database));
    return res;
  }

  sqlite3_bind_int(stmt, 1, search_order->item);
  sqlite3_bind_double(stmt, 2, search_order->unitPrice);
  sqlite3_bind_int(stmt, 3, search_order->userID);

  res = sqlite3_step(stmt);
  if (res == SQLITE_ROW) {
    int orderID = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return orderID;
  }

  fprintf(stderr, "No matching order found with a price that is lower.\n");
  sqlite3_finalize(stmt);
  return -1;  // Return -1 if no matching order is found
}

int get_item_all_orders(sqlite3* database, int item, order** orders_out, int* count_out) {
  const char* sql =
      "SELECT orderID, item, buyOrSell, quantity, unitPrice, userID, created_at "
      "FROM orders WHERE item = ?;";
  sqlite3_stmt* stmt = NULL;

  int res = sqlite3_prepare_v2(database, sql, -1, &stmt, NULL);
  if (res != SQLITE_OK) {
    fprintf(stderr, "Failed to create the get_item_all_orders statement: %s\n",
            sqlite3_errmsg(database));
    return res;
  }

  sqlite3_bind_int(stmt, 1, item);

  int capacity = 10;
  int count = 0;
  order* orders = (order*)malloc(sizeof(order) * capacity);
  if (orders == NULL) {
    fprintf(stderr, "Unable to allocate memory for orders.\n");
    sqlite3_finalize(stmt);
    return SQLITE_NOMEM;
  }

  while ((res = sqlite3_step(stmt)) == SQLITE_ROW) {
    if (count >= capacity) {
      capacity *= 2;
      order* temp = (order*)realloc(orders, sizeof(order) * capacity);
      if (temp == NULL) {
        fprintf(stderr, "Unable to reallocate memory for orders.\n");
        free(orders);
        sqlite3_finalize(stmt);
        return SQLITE_NOMEM;
      }
      orders = temp;
    }

    orders[count].orderID = sqlite3_column_int(stmt, 0);
    orders[count].item = sqlite3_column_int(stmt, 1);
    orders[count].buyOrSell = sqlite3_column_int(stmt, 2);
    orders[count].quantity = sqlite3_column_int(stmt, 3);
    orders[count].unitPrice = sqlite3_column_double(stmt, 4);
    orders[count].userID = sqlite3_column_int(stmt, 5);
    const unsigned char* created_at = sqlite3_column_text(stmt, 6);
    orders[count].created_at = strdup((const char*)created_at);

    count++;
  }

  sqlite3_finalize(stmt);

  *orders_out = orders;
  *count_out = count;

  return SQLITE_OK;
}

int update_order(sqlite3* database, const order* updated_order) {
  const char* sql =
      "UPDATE orders SET item = ?, buyOrSell = ?, quantity = ?, unitPrice = ?, userID = ? "
      "WHERE orderID = ?;";
  sqlite3_stmt* stmt = NULL;

  int res = sqlite3_prepare_v2(database, sql, -1, &stmt, NULL);
  if (res != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare the update_order statement: %s\n",
            sqlite3_errmsg(database));
    return res;
  }

  res = sqlite3_bind_int(stmt, 1, updated_order->item);
  if (res != SQLITE_OK) goto fail;
  res = sqlite3_bind_int(stmt, 2, updated_order->buyOrSell);
  if (res != SQLITE_OK) goto fail;
  res = sqlite3_bind_int(stmt, 3, updated_order->quantity);
  if (res != SQLITE_OK) goto fail;
  res = sqlite3_bind_double(stmt, 4, updated_order->unitPrice);
  if (res != SQLITE_OK) goto fail;
  res = sqlite3_bind_int(stmt, 5, updated_order->userID);
  if (res != SQLITE_OK) goto fail;
  res = sqlite3_bind_int(stmt, 6, updated_order->orderID);
  if (res != SQLITE_OK) goto fail;

  res = sqlite3_step(stmt);
  if (res != SQLITE_DONE) {
    fprintf(stderr, "Failed to execute the update_order statement: %s\n",
            sqlite3_errmsg(database));
    sqlite3_finalize(stmt);
    return res;
  }

  sqlite3_finalize(stmt);
  return SQLITE_OK;

fail:
  fprintf(stderr, "Failed to bind value for update_order: %s\n", sqlite3_errmsg(database));
  sqlite3_finalize(stmt);
  return res;
}

