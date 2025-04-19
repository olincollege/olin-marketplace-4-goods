#include <criterion/criterion.h>

#include "../src/command.h"

Test(test_command_db, test_open_db) {
  sqlite3* database = NULL;
  int res = open_db(&database);
  cr_assert_eq(res, 0, "Expected open_db to return 0, but got %d", res);
  close_db(database);
}
