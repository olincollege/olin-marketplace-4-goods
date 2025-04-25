#include <criterion/criterion.h>

#include "../src/command.h"

Test(test_command_db, test_open_db) {
  sqlite3* database = NULL;
  int res = open_db(&database);
  cr_assert_eq(res, 0, "Expected open_db to return 0, but got %d", res);
  close_db(database);
}

Test(test_command_db, test_buy) {
  sqlite3* database = NULL;
  int res = open_db(&database);
  cr_assert_eq(res, 0, "Expected open_db to return 0, but got %d", res);

  res = init_db(database);
  cr_assert_eq(res, 0, "Expected init_db to return 0, but got %d", res);

  // Create a mock user
  user new_user = {
      .userID = 1,
      .name = "Test User",
      .OMG = 100,
      .DOGE = 200,
      .BTC = 50,
      .ETH = 75,
  };

  res = insert_user(database, &new_user);
  cr_assert_eq(res, 0, "Expected insert_user to return 0, but got %d", res);

  order* test_order = create_order(1, 1, 10, 5.0, 1);
  cr_assert_not_null(
      test_order,
      "Expected create_order to return a valid order, but got NULL");

  int buy_res = buy(database, test_order);
  cr_assert_eq(buy_res, 0, "Expected buy to return 0, but got %d", buy_res);

  dump_database(
      database);  // comment this out if don't want to see the database info

  free_order(test_order);
  close_db(database);
}
