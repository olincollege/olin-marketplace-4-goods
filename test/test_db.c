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
