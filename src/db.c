#include "db.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

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
      "username TEXT UNIQUE NOT NULL, "
      "password TEXT NOT NULL, "
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
  // Retrieve user to check balance
  user updated_user;
  int res = get_user(database, new_order->userID, &updated_user);
  if (res != SQLITE_OK) {
    fprintf(stderr, "Failed to retrieve user for balance check: %s\n",
            sqlite3_errmsg(database));
    return res;
  }

  // Check if the user has sufficient balance for the order
  if (new_order->buyOrSell == 0) {  // Buy order
    int cost = new_order->quantity * new_order->unitPrice;
    if (updated_user.OMG < cost) {
      fprintf(stderr, "Insufficient OMG balance for buy order.\n");
      return SQLITE_ERROR;
    }
  } else if (new_order->buyOrSell == 1) {  // Sell order
    switch (new_order->item) {
      case 1:  // DOGE
        if (updated_user.DOGE < new_order->quantity) {
          fprintf(stderr, "Insufficient DOGE balance for sell order.\n");
          return SQLITE_ERROR;
        }
        break;
      case 2:  // BTC
        if (updated_user.BTC < new_order->quantity) {
          fprintf(stderr, "Insufficient BTC balance for sell order.\n");
          return SQLITE_ERROR;
        }
        break;
      case 3:  // ETH
        if (updated_user.ETH < new_order->quantity) {
          fprintf(stderr, "Insufficient ETH balance for sell order.\n");
          return SQLITE_ERROR;
        }
        break;
      default:
        fprintf(stderr, "Unknown item type: %d\n", new_order->item);
        return SQLITE_ERROR;
    }
  }
  // Insert the order into the database
  const char* sql =
      "INSERT INTO orders (item, buyOrSell, quantity, unitPrice, userID) "
      "VALUES (?, ?, ?, ?, ?);";

  sqlite3_stmt* stmt = NULL;
  res = sqlite3_prepare_v2(database, sql, -1, &stmt, NULL);
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

  // Update user balance based on the order type
  if (new_order->buyOrSell == 0) {  // Buy order
    int cost = new_order->quantity * new_order->unitPrice;
    updated_user.OMG -= cost;
  } else if (new_order->buyOrSell == 1) {  // Sell order
    switch (new_order->item) {
      case 1:  // DOGE
        updated_user.DOGE -= new_order->quantity;
        break;
      case 2:  // BTC
        updated_user.BTC -= new_order->quantity;
        break;
      case 3:  // ETH
        updated_user.ETH -= new_order->quantity;
        break;
    }
  }

  res = update_user_balance(database, &updated_user);
  if (res != SQLITE_OK) {
    fprintf(stderr, "Failed to update user balance: %s\n",
            sqlite3_errmsg(database));
    sqlite3_finalize(stmt);
    return res;
  }

  sqlite3_finalize(stmt);
  return SQLITE_OK;
}

int insert_user(sqlite3* database, user* new_user, int* user_ID) {
  const char* sql =
      "INSERT INTO users (username, password, name, OMG, DOGE, BTC, ETH) "
      "VALUES (?, ?, ?, ?, ?, ?, ?);";

  sqlite3_stmt* stmt = NULL;
  int res = sqlite3_prepare_v2(database, sql, -1, &stmt, NULL);
  if (res != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare insert_user statement: %s\n",
            sqlite3_errmsg(database));
    return res;
  }

  res = sqlite3_bind_text(stmt, 1, new_user->username, -1, SQLITE_STATIC);
  if (res != SQLITE_OK) goto fail;
  res = sqlite3_bind_text(stmt, 2, new_user->password, -1, SQLITE_STATIC);
  if (res != SQLITE_OK) goto fail;
  res = sqlite3_bind_text(stmt, 3, new_user->name, -1, SQLITE_STATIC);
  if (res != SQLITE_OK) goto fail;
  res = sqlite3_bind_int(stmt, 4, new_user->OMG);
  if (res != SQLITE_OK) goto fail;
  res = sqlite3_bind_int(stmt, 5, new_user->DOGE);
  if (res != SQLITE_OK) goto fail;
  res = sqlite3_bind_int(stmt, 6, new_user->BTC);
  if (res != SQLITE_OK) goto fail;
  res = sqlite3_bind_int(stmt, 7, new_user->ETH);
  if (res != SQLITE_OK) goto fail;

  res = sqlite3_step(stmt);
  if (res != SQLITE_DONE) {
    fprintf(stderr, "Failed to execute insert_user statement: %s\n",
            sqlite3_errmsg(database));
    sqlite3_finalize(stmt);
    return res;
  }

  *user_ID = sqlite3_last_insert_rowid(database);

  sqlite3_finalize(stmt);
  return SQLITE_OK;

fail:
  fprintf(stderr, "Failed to bind value for insert_user: %s\n",
          sqlite3_errmsg(database));
  sqlite3_finalize(stmt);
  return res;
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
      "SELECT userID, username, password, name, OMG, DOGE, BTC, ETH FROM users "
      "WHERE userID = ?;";
  sqlite3_stmt* stmt = NULL;

  int res = sqlite3_prepare_v2(database, sql, -1, &stmt, NULL);
  if (res != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare the get_user statement: %s\n",
            sqlite3_errmsg(database));
    return res;
  }

  res = sqlite3_bind_int(stmt, 1, userID);
  if (res != SQLITE_OK) {
    fprintf(stderr, "Failed to bind userID for get_user: %s\n",
            sqlite3_errmsg(database));
    sqlite3_finalize(stmt);
    return res;
  }

  res = sqlite3_step(stmt);
  if (res == SQLITE_ROW) {
    user_out->userID = sqlite3_column_int(stmt, 0);

    const unsigned char* username = sqlite3_column_text(stmt, 1);
    user_out->username = strdup((const char*)username);

    const unsigned char* password = sqlite3_column_text(stmt, 2);
    user_out->password = strdup((const char*)password);

    const unsigned char* name = sqlite3_column_text(stmt, 3);
    user_out->name = strdup((const char*)name);

    user_out->OMG = sqlite3_column_int(stmt, 4);
    user_out->DOGE = sqlite3_column_int(stmt, 5);
    user_out->BTC = sqlite3_column_int(stmt, 6);
    user_out->ETH = sqlite3_column_int(stmt, 7);

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

int get_item_all_orders(sqlite3* database, int item, order** buy_orders_out,
                        int* buy_count_out, order** sell_orders_out,
                        int* sell_count_out) {
  const char* buy_sql =
      "SELECT orderID, item, buyOrSell, quantity, unitPrice, userID, "
      "created_at "
      "FROM orders WHERE item = ? AND buyOrSell = 0 "
      "ORDER BY unitPrice DESC LIMIT 5;";
  const char* sell_sql =
      "SELECT orderID, item, buyOrSell, quantity, unitPrice, userID, "
      "created_at "
      "FROM orders WHERE item = ? AND buyOrSell = 1 "
      "ORDER BY unitPrice ASC LIMIT 5;";

  sqlite3_stmt* stmt = NULL;
  int res;

  // Fetch top 5 buy orders
  res = sqlite3_prepare_v2(database, buy_sql, -1, &stmt, NULL);
  if (res != SQLITE_OK) {
    fprintf(stderr, "Failed to create the buy orders statement: %s\n",
            sqlite3_errmsg(database));
    return res;
  }

  sqlite3_bind_int(stmt, 1, item);

  int buy_capacity = 5;
  int buy_count = 0;
  order* buy_orders = (order*)malloc(sizeof(order) * buy_capacity);
  if (buy_orders == NULL) {
    fprintf(stderr, "Unable to allocate memory for buy orders.\n");
    sqlite3_finalize(stmt);
    return SQLITE_NOMEM;
  }

  while ((res = sqlite3_step(stmt)) == SQLITE_ROW) {
    buy_orders[buy_count].orderID = sqlite3_column_int(stmt, 0);
    buy_orders[buy_count].item = sqlite3_column_int(stmt, 1);
    buy_orders[buy_count].buyOrSell = sqlite3_column_int(stmt, 2);
    buy_orders[buy_count].quantity = sqlite3_column_int(stmt, 3);
    buy_orders[buy_count].unitPrice = sqlite3_column_double(stmt, 4);
    buy_orders[buy_count].userID = sqlite3_column_int(stmt, 5);
    const unsigned char* created_at = sqlite3_column_text(stmt, 6);
    buy_orders[buy_count].created_at = strdup((const char*)created_at);

    buy_count++;
  }

  sqlite3_finalize(stmt);

  // Fetch top 5 sell orders
  res = sqlite3_prepare_v2(database, sell_sql, -1, &stmt, NULL);
  if (res != SQLITE_OK) {
    fprintf(stderr, "Failed to create the sell orders statement: %s\n",
            sqlite3_errmsg(database));
    free(buy_orders);
    return res;
  }

  sqlite3_bind_int(stmt, 1, item);

  int sell_capacity = 5;
  int sell_count = 0;
  order* sell_orders = (order*)malloc(sizeof(order) * sell_capacity);
  if (sell_orders == NULL) {
    fprintf(stderr, "Unable to allocate memory for sell orders.\n");
    free(buy_orders);
    sqlite3_finalize(stmt);
    return SQLITE_NOMEM;
  }

  while ((res = sqlite3_step(stmt)) == SQLITE_ROW) {
    sell_orders[sell_count].orderID = sqlite3_column_int(stmt, 0);
    sell_orders[sell_count].item = sqlite3_column_int(stmt, 1);
    sell_orders[sell_count].buyOrSell = sqlite3_column_int(stmt, 2);
    sell_orders[sell_count].quantity = sqlite3_column_int(stmt, 3);
    sell_orders[sell_count].unitPrice = sqlite3_column_double(stmt, 4);
    sell_orders[sell_count].userID = sqlite3_column_int(stmt, 5);
    const unsigned char* created_at = sqlite3_column_text(stmt, 6);
    sell_orders[sell_count].created_at = strdup((const char*)created_at);

    sell_count++;
  }

  sqlite3_finalize(stmt);

  *buy_orders_out = buy_orders;
  *buy_count_out = buy_count;
  *sell_orders_out = sell_orders;
  *sell_count_out = sell_count;

  return SQLITE_OK;
}

int update_order(sqlite3* database, const order* updated_order) {
  const char* sql =
      "UPDATE orders SET item = ?, buyOrSell = ?, quantity = ?, unitPrice = ?, "
      "userID = ? "
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
  fprintf(stderr, "Failed to bind value for update_order: %s\n",
          sqlite3_errmsg(database));
  sqlite3_finalize(stmt);
  return res;
}

int update_user_balance(sqlite3* database, const user* updated_user) {
  const char* sql =
      "UPDATE users SET OMG = ?, DOGE = ?, BTC = ?, ETH = ? WHERE userID = ?;";
  sqlite3_stmt* stmt = NULL;

  int res = sqlite3_prepare_v2(database, sql, -1, &stmt, NULL);
  if (res != SQLITE_OK) {
    fprintf(stderr,
            " Unable to prepare the update_user_balance statement: %s/n",
            sqlite3_errmsg(database));
    return res;
  }

  res = sqlite3_bind_int(stmt, 1, updated_user->OMG);
  if (res != SQLITE_OK) goto fail;
  res = sqlite3_bind_int(stmt, 2, updated_user->DOGE);
  if (res != SQLITE_OK) goto fail;
  res = sqlite3_bind_int(stmt, 3, updated_user->BTC);
  if (res != SQLITE_OK) goto fail;
  res = sqlite3_bind_int(stmt, 4, updated_user->ETH);
  if (res != SQLITE_OK) goto fail;
  res = sqlite3_bind_int(stmt, 5, updated_user->userID);
  if (res != SQLITE_OK) goto fail;

  res = sqlite3_step(stmt);
  if (res != SQLITE_DONE) {
    fprintf(stderr, "Failed to execute the update_user_balance statement: %s\n",
            sqlite3_errmsg(database));
    sqlite3_finalize(stmt);
    return res;
  }

  sqlite3_finalize(stmt);
  return SQLITE_OK;

fail:
  fprintf(stderr, "Failed to bind value for update_user_balance: %s\n",
          sqlite3_errmsg(database));
  sqlite3_finalize(stmt);
  return res;
}

int get_user_all_orders(sqlite3* database, int userID, order** orders_out,
                        int* count_out) {
  *count_out = 0;
  *orders_out = NULL;

  const char* sql =
      "SELECT orderID, item, buyOrSell, quantity, unitPrice, userID, "
      "created_at "
      "FROM orders WHERE userID = ?;";
  sqlite3_stmt* stmt = NULL;

  int res = sqlite3_prepare_v2(database, sql, -1, &stmt, NULL);
  if (res != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare the get_user_all_orders statement: %s\n",
            sqlite3_errmsg(database));
    return res;
  }

  sqlite3_bind_int(stmt, 1, userID);

  while ((sqlite3_step(stmt)) == SQLITE_ROW) {
    order new_order;
    new_order.orderID = sqlite3_column_int(stmt, 0);
    new_order.item = sqlite3_column_int(stmt, 1);
    new_order.buyOrSell = sqlite3_column_int(stmt, 2);
    new_order.quantity = sqlite3_column_int(stmt, 3);
    new_order.unitPrice = sqlite3_column_double(stmt, 4);
    new_order.userID = sqlite3_column_int(stmt, 5);
    const unsigned char* created_at = sqlite3_column_text(stmt, 6);
    new_order.created_at = strdup((const char*)created_at);

    order* temp =
        realloc(*orders_out, (size_t)(*count_out + 1) * sizeof(order));
    if (temp == NULL) {
      error_and_exit("Realloc failed");
    }
    *orders_out = temp;
    (*orders_out)[*count_out] = new_order;
    (*count_out)++;
  }

  sqlite3_finalize(stmt);

  return SQLITE_OK;
}

int get_user_inventories(sqlite3* database, user* user_out) {
  const char* sql = "SELECT OMG, DOGE, BTC, ETH FROM users WHERE userID = ?;";
  sqlite3_stmt* stmt = NULL;

  int res = sqlite3_prepare_v2(database, sql, -1, &stmt, NULL);
  if (res != SQLITE_OK) {
    fprintf(stderr,
            "Failed to prepare the get_user_inventories statement: %s\n",
            sqlite3_errmsg(database));
    return res;
  }

  sqlite3_bind_int(stmt, 1, user_out->userID);

  res = sqlite3_step(stmt);
  if (res == SQLITE_ROW) {
    user_out->OMG = sqlite3_column_int(stmt, 0);
    user_out->DOGE = sqlite3_column_int(stmt, 1);
    user_out->BTC = sqlite3_column_int(stmt, 2);
    user_out->ETH = sqlite3_column_int(stmt, 3);
    sqlite3_finalize(stmt);
    return SQLITE_OK;
  }

  fprintf(stderr, "User with ID %d not found.\n", user_out->userID);
  sqlite3_finalize(stmt);
  return SQLITE_NOTFOUND;
}

int get_user_by_username(sqlite3* database, const char* username,
                         user* user_out) {
  const char* sql =
      "SELECT userID, username, password, name, OMG, DOGE, BTC, ETH FROM users "
      "WHERE username = ?;";
  sqlite3_stmt* stmt = NULL;

  int res = sqlite3_prepare_v2(database, sql, -1, &stmt, NULL);
  if (res != SQLITE_OK) {
    fprintf(stderr,
            "Failed to prepare the get_user_by_username statement: %s\n",
            sqlite3_errmsg(database));
    return res;
  }

  res = sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
  if (res != SQLITE_OK) {
    fprintf(stderr, "Failed to bind username for get_user_by_username: %s\n",
            sqlite3_errmsg(database));
    sqlite3_finalize(stmt);
    return res;
  }

  res = sqlite3_step(stmt);
  if (res == SQLITE_ROW) {
    user_out->userID = sqlite3_column_int(stmt, 0);

    const unsigned char* db_username = sqlite3_column_text(stmt, 1);
    user_out->username = strdup((const char*)db_username);

    const unsigned char* password = sqlite3_column_text(stmt, 2);
    user_out->password = strdup((const char*)password);

    const unsigned char* name = sqlite3_column_text(stmt, 3);
    user_out->name = strdup((const char*)name);

    user_out->OMG = sqlite3_column_int(stmt, 4);
    user_out->DOGE = sqlite3_column_int(stmt, 5);
    user_out->BTC = sqlite3_column_int(stmt, 6);
    user_out->ETH = sqlite3_column_int(stmt, 7);

    sqlite3_finalize(stmt);
    return SQLITE_OK;
  }

  fprintf(stderr, "User with username '%s' not found.\n", username);
  sqlite3_finalize(stmt);
  return SQLITE_NOTFOUND;
}

int insert_archive(sqlite3* database, const order* archived_order) {
  const char* sql =
      "INSERT INTO archives (item, buyOrSell, quantity, unitPrice, userID, "
      "created_at) "
      "VALUES (?, ?, ?, ?, ?, ?);";

  sqlite3_stmt* stmt = NULL;
  int res = sqlite3_prepare_v2(database, sql, -1, &stmt, NULL);
  if (res != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare the insert_archive statement: %s\n",
            sqlite3_errmsg(database));
    return res;
  }

  sqlite3_bind_int(stmt, 1, archived_order->item);
  sqlite3_bind_int(stmt, 2, archived_order->buyOrSell);
  sqlite3_bind_int(stmt, 3, archived_order->quantity);
  sqlite3_bind_double(stmt, 4, archived_order->unitPrice);
  sqlite3_bind_int(stmt, 5, archived_order->userID);
  sqlite3_bind_text(stmt, 6, archived_order->created_at, -1, SQLITE_TRANSIENT);

  res = sqlite3_step(stmt);
  if (res != SQLITE_DONE) {
    fprintf(stderr, "Failed to execute the insert_archive statement: %s\n",
            sqlite3_errmsg(database));
    sqlite3_finalize(stmt);
    return res;
  }

  sqlite3_finalize(stmt);
  return SQLITE_OK;
}

int get_user_archived_orders(sqlite3* database, int userID, order** orders_out,
                             int* count_out) {
  const char* sql =
      "SELECT orderID, item, buyOrSell, quantity, unitPrice, userID, "
      "created_at "
      "FROM archives WHERE userID = ?;";
  sqlite3_stmt* stmt = NULL;

  int res = sqlite3_prepare_v2(database, sql, -1, &stmt, NULL);
  if (res != SQLITE_OK) {
    fprintf(stderr,
            "Failed to prepare the get_user_archived_orders statement: %s\n",
            sqlite3_errmsg(database));
    return res;
  }

  sqlite3_bind_int(stmt, 1, userID);

  int capacity = 10;
  int count = 0;
  order* orders = (order*)malloc(sizeof(order) * capacity);
  if (orders == NULL) {
    fprintf(stderr, "Unable to allocate memory for archived orders.\n");
    sqlite3_finalize(stmt);
    return SQLITE_NOMEM;
  }

  while ((sqlite3_step(stmt)) == SQLITE_ROW) {
    if (count >= capacity) {
      capacity *= 2;
      order* temp = (order*)realloc(orders, sizeof(order) * capacity);
      if (temp == NULL) {
        fprintf(stderr, "Unable to reallocate memory for archived orders.\n");
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

int assign_order_timestamp(sqlite3* database, order* order_to_update) {
  const char* sql = "SELECT CURRENT_TIMESTAMP;";
  sqlite3_stmt* stmt = NULL;

  int res = sqlite3_prepare_v2(database, sql, -1, &stmt, NULL);
  if (res != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare the select statement: %s\n",
            sqlite3_errmsg(database));
    return res;
  }

  res = sqlite3_step(stmt);
  if (res == SQLITE_ROW) {
    const unsigned char* current_timestamp = sqlite3_column_text(stmt, 0);
    order_to_update->created_at = strdup((const char*)current_timestamp);
  } else {
    fprintf(stderr, "Failed to retrieve the current timestamp: %s\n",
            sqlite3_errmsg(database));
    sqlite3_finalize(stmt);
    return res;
  }

  sqlite3_finalize(stmt);
  return SQLITE_OK;
}
