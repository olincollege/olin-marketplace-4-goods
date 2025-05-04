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

  order* test_order = create_order(1, 1, 10, 5.0, new_user.userID);
  cr_assert_not_null(
      test_order,
      "Expected create_order to return a valid order, but got NULL");

  int buy_res = buy(database, test_order);
  cr_assert_eq(buy_res, 0, "Expected buy to return 0, but got %d", buy_res);

  // dump_database(
  //     database);  // comment this out if don't want to see the database info

  free_order(test_order);
  close_db(database);
}

Test(test_command_db, test_sell) {
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

  order* test_order = create_order(1, 1, 10, 5.0, new_user.userID);
  cr_assert_not_null(
      test_order,
      "Expected create_order to return a valid order, but got NULL");

  int sell_res = sell(database, test_order);
  cr_assert_eq(sell_res, 0, "Expected sell to return 0, but got %d", sell_res);

  // dump_database(
  //     database);  // comment this out if don't want to see the database info

  free_order(test_order);
  close_db(database);
}

Test(test_command_db, test_sell_with_two_users) {
  sqlite3* database = NULL;
  int res = open_db(&database);
  cr_assert_eq(res, 0, "Expected open_db to return 0, but got %d", res);

  res = init_db(database);
  cr_assert_eq(res, 0, "Expected init_db to return 0, but got %d", res);

  // Create two mock users
  user user1 = {
      .userID = 1,
      .name = "Buyer1",
      .OMG = 100,
      .DOGE = 200,
      .BTC = 50,
      .ETH = 75,
  };

  user user2 = {
      .userID = 2,
      .name = "Seller",
      .OMG = 50,
      .DOGE = 100,
      .BTC = 25,
      .ETH = 50,
  };

  res = insert_user(database, &user1);
  cr_assert_eq(res, 0, "Expected insert_user to return 0, but got %d", res);

  res = insert_user(database, &user2);
  cr_assert_eq(res, 0, "Expected insert_user to return 0, but got %d", res);

  // User1 publishes two buy orders
  order* buy_order1 = create_order(1, BUY, 10, 10.0, user1.userID);
  cr_assert_not_null(
      buy_order1,
      "Expected create_order to return a valid order, but got NULL");

  res = buy(database, buy_order1);
  cr_assert_eq(res, 0, "Expected buy to return 0, but got %d", res);

  order* buy_order2 = create_order(1, BUY, 2, 14.5, user1.userID);
  cr_assert_not_null(
      buy_order2,
      "Expected create_order to return a valid order, but got NULL");

  res = buy(database, buy_order2);
  cr_assert_eq(res, 0, "Expected buy to return 0, but got %d", res);

  // User2 tries to post a sell order at a higher price
  order* sell_order_high = create_order(1, 0, 10, 6.0, user2.userID);
  cr_assert_not_null(
      sell_order_high,
      "Expected create_order to return a valid order, but got NULL");
  dump_database(database);

  int sell_res_high = sell(database, sell_order_high);
  cr_assert_eq(sell_res_high, 0, "Selling failed, got %d", sell_res_high);

  free_order(sell_order_high);

  // Dump database to verify the state
  dump_database(database);

  free_order(buy_order1);
  free_order(buy_order2);
  close_db(database);
}

Test(test_command_db, test_partial_match_sell_and_buy) {
  sqlite3* database = NULL;
  int res = open_db(&database);
  cr_assert_eq(res, 0, "Expected open_db to return 0, but got %d", res);

  res = init_db(database);
  cr_assert_eq(res, 0, "Expected init_db to return 0, but got %d", res);

  // Create two mock users
  user buyer = {
      .userID = 1,
      .name = "Buyer",
      .OMG = 1000,
      .DOGE = 200,
      .BTC = 50,
      .ETH = 75,
  };

  user seller = {
      .userID = 2,
      .name = "Seller",
      .OMG = 50,
      .DOGE = 100,
      .BTC = 25,
      .ETH = 50,
  };

  res = insert_user(database, &buyer);
  cr_assert_eq(res, 0, "Expected insert_user to return 0, but got %d", res);

  res = insert_user(database, &seller);
  cr_assert_eq(res, 0, "Expected insert_user to return 0, but got %d", res);

  // Buyer places a buy order for 10 units
  order* buy_order = create_order(1, BUY, 10, 10.0, buyer.userID);
  cr_assert_not_null(
      buy_order, "Expected create_order to return a valid order, but got NULL");

  res = buy(database, buy_order);
  cr_assert_eq(res, 0, "Expected buy to return 0, but got %d", res);

  // Seller places a sell order for 5 units
  order* sell_order = create_order(1, SELL, 5, 10.0, seller.userID);
  cr_assert_not_null(
      sell_order,
      "Expected create_order to return a valid order, but got NULL");
  // Dump database to verify the state before partial match
  dump_database(database);
  int sell_res = sell(database, sell_order);
  cr_assert_eq(sell_res, 0, "Expected sell to return 0, but got %d", sell_res);

  // Dump database to verify the state after partial match
  dump_database(database);
  free_order(buy_order);
  free_order(sell_order);
  close_db(database);
}
