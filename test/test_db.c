#include <criterion/criterion.h>

#include "../src/db.h"

Test(test_db, test_open_db) {
  sqlite3* database = open_database();
  cr_assert_not_null(database, "Database connection should not be NULL");
  close_database(database);
}

Test(test_db, test_close_db) {
  sqlite3* database = open_database();
  int res = close_database(database);
  cr_assert_eq(res, SQLITE_OK,
               "Expected close_database to return SQLITE_OK, but got %d", res);
}

Test(test_db, test_drop_all_tables) {
  // Open database
  sqlite3* database = open_database();
  cr_assert_not_null(database, "Database connection should not be NULL");

  // Create tables to ensure they exist
  int res = create_tables(database);
  cr_assert_eq(res, SQLITE_OK, "create_tables failed: %d", res);

  // Drop all tables
  res = drop_all_tables(database);
  cr_assert_eq(res, SQLITE_OK, "drop_all_tables failed: %d", res);

  // Helper lambda-like macro to check table non-existence
#define CHECK_TABLE_NOT_EXISTS(table_name)                                  \
  do {                                                                      \
    const char* sql =                                                       \
        "SELECT name FROM sqlite_master WHERE type='table' AND "            \
        "name='" table_name "';";                                           \
    sqlite3_stmt* stmt;                                                     \
    res = sqlite3_prepare_v2(database, sql, -1, &stmt, NULL);               \
    cr_assert_eq(res, SQLITE_OK, "Failed to prepare statement for %s: %s",  \
                 table_name, sqlite3_errmsg(database));                     \
    res = sqlite3_step(stmt);                                               \
    cr_assert_eq(res, SQLITE_DONE, "Table '%s' still exists.", table_name); \
    sqlite3_finalize(stmt);                                                 \
  } while (0)

  CHECK_TABLE_NOT_EXISTS("users");
  CHECK_TABLE_NOT_EXISTS("orders");
  CHECK_TABLE_NOT_EXISTS("archives");

  close_database(database);
}

Test(test_db, test_create_tables_function) {
  // Open database
  sqlite3* database = open_database();
  drop_all_tables(database);  // Reset tables

  // Call the function being tested
  int res = create_tables(database);
  cr_assert_eq(res, SQLITE_OK, "create_tables failed: %d", res);

  // Helper lambda-like macro to check table existence
#define CHECK_TABLE_EXISTS(table_name)                                       \
  do {                                                                       \
    const char* sql =                                                        \
        "SELECT name FROM sqlite_master WHERE type='table' AND "             \
        "name='" table_name "';";                                            \
    sqlite3_stmt* stmt;                                                      \
    res = sqlite3_prepare_v2(database, sql, -1, &stmt, NULL);                \
    cr_assert_eq(res, SQLITE_OK, "Failed to prepare statement for %s: %s",   \
                 table_name, sqlite3_errmsg(database));                      \
    res = sqlite3_step(stmt);                                                \
    cr_assert_eq(res, SQLITE_ROW, "Table '%s' does not exist.", table_name); \
    sqlite3_finalize(stmt);                                                  \
  } while (0)

  CHECK_TABLE_EXISTS("users");
  CHECK_TABLE_EXISTS("orders");
  CHECK_TABLE_EXISTS("archives");

  close_database(database);
}

Test(test_orders, test_insert_order) {
  // Open database
  sqlite3* database = open_database();
  cr_assert_not_null(database, "Database connection should not be NULL");

  // Reset tables
  drop_all_tables(database);
  create_tables(database);

  // Create a mock order
  order new_order = {
      .item = COIN_BTC,
      .buyOrSell = BUY,
      .quantity = 10,
      .unitPrice = 100.50,
      .userID = 1,
  };

  // Insert a user to satisfy the foreign key constraint
  const char* insert_user_sql =
      "INSERT INTO users (name) VALUES ('Test User');";
  char* errMsg = NULL;
  int res = sqlite3_exec(database, insert_user_sql, NULL, NULL, &errMsg);
  cr_assert_eq(res, SQLITE_OK, "Failed to insert user: %s", errMsg);
  sqlite3_free(errMsg);

  // Call the function being tested
  res = insert_order(database, &new_order);
  cr_assert_eq(res, SQLITE_OK, "insert_order failed: %d", res);

  // Verify the order was inserted
  const char* verify_sql =
      "SELECT item, buyOrSell, quantity, unitPrice, userID FROM orders;";
  sqlite3_stmt* stmt = NULL;
  res = sqlite3_prepare_v2(database, verify_sql, -1, &stmt, NULL);
  cr_assert_eq(res, SQLITE_OK, "Failed to prepare verification statement: %s",
               sqlite3_errmsg(database));

  res = sqlite3_step(stmt);
  cr_assert_eq(res, SQLITE_ROW, "No order was inserted into the database.");

  cr_assert_eq(sqlite3_column_int(stmt, 0), new_order.item,
               "Inserted item does not match.");
  cr_assert_eq(sqlite3_column_int(stmt, 1), new_order.buyOrSell,
               "Inserted buyOrSell does not match.");
  cr_assert_eq(sqlite3_column_int(stmt, 2), new_order.quantity,
               "Inserted quantity does not match.");
  cr_assert_float_eq(sqlite3_column_double(stmt, 3), new_order.unitPrice, 0.001,
                     "Inserted unitPrice does not match.");
  cr_assert_eq(sqlite3_column_int(stmt, 4), new_order.userID,
               "Inserted userID does not match.");

  sqlite3_finalize(stmt);
  close_database(database);
}
Test(test_users, test_insert_user) {
  // Open database
  sqlite3* database = open_database();
  cr_assert_not_null(database, "Database connection should not be NULL");

  // Reset tables
  drop_all_tables(database);
  create_tables(database);

  // Create a mock user
  user new_user = {
      .userID = 1,
      .name = "Test User",
      .OMG = 100,
      .DOGE = 200,
      .BTC = 50,
      .ETH = 75,
  };

  // Call the function being tested
  int res = insert_user(database, &new_user);
  cr_assert_eq(res, SQLITE_OK, "insert_user failed: %d", res);

  // Verify the user was inserted
  const char* verify_sql =
      "SELECT userID, name, OMG, DOGE, BTC, ETH FROM users WHERE userID = ?;";
  sqlite3_stmt* stmt = NULL;
  res = sqlite3_prepare_v2(database, verify_sql, -1, &stmt, NULL);
  cr_assert_eq(res, SQLITE_OK, "Failed to prepare verification statement: %s",
               sqlite3_errmsg(database));

  res = sqlite3_bind_int(stmt, 1, new_user.userID);
  cr_assert_eq(res, SQLITE_OK, "Failed to bind user ID: %s",
               sqlite3_errmsg(database));

  res = sqlite3_step(stmt);
  cr_assert_eq(res, SQLITE_ROW, "No user was inserted into the database.");

  cr_assert_eq(sqlite3_column_int(stmt, 0), new_user.userID,
               "Inserted user ID does not match.");
  cr_assert_str_eq((const char*)sqlite3_column_text(stmt, 1), new_user.name,
                   "Inserted user name does not match.");
  cr_assert_eq(sqlite3_column_int(stmt, 2), new_user.OMG,
               "Inserted OMG balance does not match.");
  cr_assert_eq(sqlite3_column_int(stmt, 3), new_user.DOGE,
               "Inserted DOGE balance does not match.");
  cr_assert_eq(sqlite3_column_int(stmt, 4), new_user.BTC,
               "Inserted BTC balance does not match.");
  cr_assert_eq(sqlite3_column_int(stmt, 5), new_user.ETH,
               "Inserted ETH balance does not match.");

  sqlite3_finalize(stmt);
  close_database(database);
}
