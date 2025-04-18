#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

#include "command.c"

// Opens a database for use
sqlite3* open_database(const char* filename) {
  sqlite3* db;
  if (sqlite3_open(filename, &db) != SQLITE_OK) {
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    return NULL;
  }
  return db;
}

// Closes the database
void close_database(sqlite3* db) {
  if (db) sqlite3_close(db);
}

// Creates a basic table for the inventory
int create_table(sqlite3* db) {
  const char* sql =
      "CREATE TABLE IF NOT EXISTS inventory ("
      "id,"
      "omg,"
      "coin1"
      "coin2"
      "coin3);";
  char* errMsg = 0;
  int rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Create table error: %s\n", errMsg);
    sqlite3_free(errMsg);
  }
  return rc;
}

// Insert a user records into the inventory
int insert_item(sqlite3* db, int id, int omg, int coin1, int coin2, int coin3) {
  char sql[256];
  snprintf(sql, sizeof(sql),
           "INSERT INTO inventory (id, omg, coin1, coin2, coin3) VALUES (%d, "
           "%d, %d, %d, %d);",
           id, omg, coin1, coin2, coin3);
  return sqlite3_exec(db, sql, 0, 0, 0);
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
int select_all(sqlite3* db) {
  const char* sql = "SELECT * FROM inventory;";
  return sqlite3_exec(db, sql, select_callback, 0, 0);
}

// Update inventory
int update_item(sqlite3* db, int id, int omg, int coin1, int coin2, int coin3) {
  char sql[256];
  snprintf(sql, sizeof(sql),
           "UPDATE inventory SET omg = %d, coin1 = %d, coin2 = %d, coin3 = %d "
           "WHERE id = %d;",
           omg, coin1, coin2, coin3, id);
  return sqlite3_exec(db, sql, 0, 0, 0);
}

// Delete user inventory
int delete_item(sqlite3* db, int id) {
  char sql[128];
  snprintf(sql, sizeof(sql), "DELETE FROM inventory WHERE id = %d;", id);
  return sqlite3_exec(db, sql, 0, 0, 0);
}

int insert_order(sqlite3* db, order* new_order) {
  char sql[256];
  snprintf(sql, sizeof(sql),
           "INSERT INTO orders (item, buyOrSell, quantity, unitPrice, userID) "
           "VALUES (%d, %d, %d, %.2f, %d);",
           new_order->item, new_order->buyOrSell, new_order->quantity,
           new_order->unitPrice, new_order->userID);
  return sqlite3_exec(db, sql, 0, 0, 0);
}
