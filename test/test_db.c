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

Test(test_db, test_create_tables_function) {
  // Open database
  sqlite3* db = open_database();

  // Call the function being tested
  int res = create_tables(db);
  cr_assert_eq(res, SQLITE_OK, "create_tables failed: %d", res);

// Helper lambda-like macro to check table existence
#define CHECK_TABLE_EXISTS(table_name)                                       \
  do {                                                                       \
    const char* sql =                                                        \
        "SELECT name FROM sqlite_master WHERE type='table' AND "             \
        "name='" table_name "';";                                            \
    sqlite3_stmt* stmt;                                                      \
    res = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);                      \
    cr_assert_eq(res, SQLITE_OK, "Failed to prepare statement for %s: %s",   \
                 table_name, sqlite3_errmsg(db));                            \
    res = sqlite3_step(stmt);                                                \
    cr_assert_eq(res, SQLITE_ROW, "Table '%s' does not exist.", table_name); \
    sqlite3_finalize(stmt);                                                  \
  } while (0)

  CHECK_TABLE_EXISTS("users");
  CHECK_TABLE_EXISTS("orders");
  CHECK_TABLE_EXISTS("archives");

  close_database(db);
}
