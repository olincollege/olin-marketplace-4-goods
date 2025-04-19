#include <criterion/criterion.h>

#include "../src/command.h"

Test(sample_test_suite, sample_test_case) {
  int a = 5;
  int b = 5;

  cr_assert(a == b, "Expected %d to be equal to %d", a, b);
}

Test(test_command_db, test_open_db) {
  sqlite3* db = NULL;
  int rc = open_db(&db);
  cr_assert_eq(rc, 0, "Expected open_db to return 0, but got %d", rc);
  close_db(db);
}
