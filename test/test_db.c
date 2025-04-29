#include <criterion/criterion.h>
#include <stdio.h>
#include <unistd.h>

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

Test(test_orders, test_delete_order) {
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

  // Insert the order
  res = insert_order(database, &new_order);
  cr_assert_eq(res, SQLITE_OK, "insert_order failed: %d", res);

  // Verify the order was inserted
  const char* verify_insert_sql = "SELECT orderID FROM orders;";
  sqlite3_stmt* stmt = NULL;
  res = sqlite3_prepare_v2(database, verify_insert_sql, -1, &stmt, NULL);
  cr_assert_eq(res, SQLITE_OK, "Failed to prepare verification statement: %s",
               sqlite3_errmsg(database));

  res = sqlite3_step(stmt);
  cr_assert_eq(res, SQLITE_ROW, "No order was inserted into the database.");

  int orderID = sqlite3_column_int(stmt, 0);
  sqlite3_finalize(stmt);

  // Delete the order
  res = delete_order(database, orderID);
  cr_assert_eq(res, SQLITE_OK, "delete_order failed: %d", res);

  // Verify the order was deleted
  const char* verify_delete_sql =
      "SELECT orderID FROM orders WHERE orderID = ?;";
  res = sqlite3_prepare_v2(database, verify_delete_sql, -1, &stmt, NULL);
  cr_assert_eq(res, SQLITE_OK, "Failed to prepare verification statement: %s",
               sqlite3_errmsg(database));

  res = sqlite3_bind_int(stmt, 1, orderID);
  cr_assert_eq(res, SQLITE_OK, "Failed to bind order ID: %s",
               sqlite3_errmsg(database));

  res = sqlite3_step(stmt);
  cr_assert_eq(res, SQLITE_DONE, "Order was not deleted from the database.");

  sqlite3_finalize(stmt);
  close_database(database);
}
Test(test_orders, test_get_item_all_orders) {
  // Open database
  sqlite3* database = open_database();
  cr_assert_not_null(database, "Database connection should not be NULL");

  // Reset tables
  drop_all_tables(database);
  create_tables(database);

  // Insert a user (required for foreign key constraint)
  const char* insert_user_sql =
      "INSERT INTO users (name) VALUES ('Test User');";
  char* errMsg = NULL;
  int res = sqlite3_exec(database, insert_user_sql, NULL, NULL, &errMsg);
  cr_assert_eq(res, SQLITE_OK, "Failed to insert user: %s", errMsg);
  sqlite3_free(errMsg);

  // Insert multiple orders (more than 5 for both buy and sell)
  order orders[] = {
      {.item = COIN_BTC,
       .buyOrSell = BUY,
       .quantity = 5,
       .unitPrice = 100.0,
       .userID = 1},
      {.item = COIN_BTC,
       .buyOrSell = BUY,
       .quantity = 10,
       .unitPrice = 110.0,
       .userID = 1},
      {.item = COIN_BTC,
       .buyOrSell = BUY,
       .quantity = 15,
       .unitPrice = 120.0,
       .userID = 1},
      {.item = COIN_BTC,
       .buyOrSell = BUY,
       .quantity = 20,
       .unitPrice = 130.0,
       .userID = 1},
      {.item = COIN_BTC,
       .buyOrSell = BUY,
       .quantity = 25,
       .unitPrice = 140.0,
       .userID = 1},
      {.item = COIN_BTC,
       .buyOrSell = BUY,
       .quantity = 30,
       .unitPrice = 150.0,
       .userID = 1},
      {.item = COIN_BTC,
       .buyOrSell = SELL,
       .quantity = 3,
       .unitPrice = 90.0,
       .userID = 1},
      {.item = COIN_BTC,
       .buyOrSell = SELL,
       .quantity = 6,
       .unitPrice = 80.0,
       .userID = 1},
      {.item = COIN_BTC,
       .buyOrSell = SELL,
       .quantity = 9,
       .unitPrice = 70.0,
       .userID = 1},
      {.item = COIN_BTC,
       .buyOrSell = SELL,
       .quantity = 12,
       .unitPrice = 60.0,
       .userID = 1},
      {.item = COIN_BTC,
       .buyOrSell = SELL,
       .quantity = 15,
       .unitPrice = 50.0,
       .userID = 1},
      {.item = COIN_BTC,
       .buyOrSell = SELL,
       .quantity = 18,
       .unitPrice = 40.0,
       .userID = 1},
  };

  for (int i = 0; i < sizeof(orders) / sizeof(orders[0]); i++) {
    res = insert_order(database, &orders[i]);
    cr_assert_eq(res, SQLITE_OK, "Failed to insert order %d: %d", i + 1, res);
  }

  // Test get_item_all_orders
  order* buy_orders = NULL;
  int buy_count = 0;
  order* sell_orders = NULL;
  int sell_count = 0;

  res = get_item_all_orders(database, COIN_BTC, &buy_orders, &buy_count,
                            &sell_orders, &sell_count);
  cr_assert_eq(res, SQLITE_OK, "get_item_all_orders failed: %d", res);

  // Verify buy orders
  cr_assert_eq(buy_count, 5, "Expected 5 buy orders, but got %d", buy_count);
  cr_assert_not_null(buy_orders, "Returned buy order list should not be NULL");
  printf("Top 5 Buy Orders:\n");
  for (int i = 0; i < buy_count; i++) {
    printf(
        "Buy Order %d: orderID=%d, item=%d, buyOrSell=%d, quantity=%d, "
        "unitPrice=%.2f, userID=%d, created_at=%s\n",
        i + 1, buy_orders[i].orderID, buy_orders[i].item,
        buy_orders[i].buyOrSell, buy_orders[i].quantity,
        buy_orders[i].unitPrice, buy_orders[i].userID,
        buy_orders[i].created_at);
    free(buy_orders[i].created_at);
  }
  free(buy_orders);

  // Verify sell orders
  cr_assert_eq(sell_count, 5, "Expected 5 sell orders, but got %d", sell_count);
  cr_assert_not_null(sell_orders,
                     "Returned sell order list should not be NULL");
  printf("Top 5 Sell Orders:\n");
  for (int i = 0; i < sell_count; i++) {
    printf(
        "Sell Order %d: orderID=%d, item=%d, buyOrSell=%d, quantity=%d, "
        "unitPrice=%.2f, userID=%d, created_at=%s\n",
        i + 1, sell_orders[i].orderID, sell_orders[i].item,
        sell_orders[i].buyOrSell, sell_orders[i].quantity,
        sell_orders[i].unitPrice, sell_orders[i].userID,
        sell_orders[i].created_at);
    free(sell_orders[i].created_at);
  }
  free(sell_orders);

  close_database(database);
}

Test(test_orders, test_update_order) {
  // Open database
  sqlite3* database = open_database();
  cr_assert_not_null(database, "Database connection should not be NULL");

  // Reset tables
  drop_all_tables(database);
  create_tables(database);

  // Insert a user to satisfy foreign key constraint
  const char* insert_user_sql =
      "INSERT INTO users (name) VALUES ('Test User');";
  char* errMsg = NULL;
  int res = sqlite3_exec(database, insert_user_sql, NULL, NULL, &errMsg);
  cr_assert_eq(res, SQLITE_OK, "Failed to insert user: %s", errMsg);
  sqlite3_free(errMsg);

  // Insert an order
  order new_order = {
      .item = COIN_BTC,
      .buyOrSell = BUY,
      .quantity = 10,
      .unitPrice = 100.0,
      .userID = 1,
  };
  res = insert_order(database, &new_order);
  cr_assert_eq(res, SQLITE_OK, "insert_order failed: %d", res);

  // Retrieve the inserted order's ID
  const char* select_sql = "SELECT orderID FROM orders;";
  sqlite3_stmt* stmt = NULL;
  res = sqlite3_prepare_v2(database, select_sql, -1, &stmt, NULL);
  cr_assert_eq(res, SQLITE_OK, "Failed to prepare select statement: %s",
               sqlite3_errmsg(database));

  res = sqlite3_step(stmt);
  cr_assert_eq(res, SQLITE_ROW, "No order inserted to fetch ID.");

  int orderID = sqlite3_column_int(stmt, 0);
  sqlite3_finalize(stmt);

  // Update the order
  order updated_order = {
      .orderID = orderID,
      .item = COIN_ETH,    // Change to ETH
      .buyOrSell = SELL,   // Change to SELL
      .quantity = 20,      // Update quantity
      .unitPrice = 150.0,  // Update price
      .userID = 1,         // Same user
  };

  res = update_order(database, &updated_order);
  cr_assert_eq(res, SQLITE_OK, "update_order failed: %d", res);

  // Verify the update
  const char* verify_sql =
      "SELECT item, buyOrSell, quantity, unitPrice, userID FROM orders WHERE "
      "orderID = ?;";
  res = sqlite3_prepare_v2(database, verify_sql, -1, &stmt, NULL);
  cr_assert_eq(res, SQLITE_OK, "Failed to prepare verification select: %s",
               sqlite3_errmsg(database));

  res = sqlite3_bind_int(stmt, 1, orderID);
  cr_assert_eq(res, SQLITE_OK, "Failed to bind order ID: %s",
               sqlite3_errmsg(database));

  res = sqlite3_step(stmt);
  cr_assert_eq(res, SQLITE_ROW, "No order found with given ID after update.");

  cr_assert_eq(sqlite3_column_int(stmt, 0), updated_order.item,
               "Updated item does not match.");
  cr_assert_eq(sqlite3_column_int(stmt, 1), updated_order.buyOrSell,
               "Updated buyOrSell does not match.");
  cr_assert_eq(sqlite3_column_int(stmt, 2), updated_order.quantity,
               "Updated quantity does not match.");
  cr_assert_float_eq(sqlite3_column_double(stmt, 3), updated_order.unitPrice,
                     0.001, "Updated unitPrice does not match.");
  cr_assert_eq(sqlite3_column_int(stmt, 4), updated_order.userID,
               "Updated userID does not match.");

  sqlite3_finalize(stmt);
  close_database(database);
}

Test(test_orders, test_find_matching_buy) {
  // Open database
  sqlite3* database = open_database();
  cr_assert_not_null(database, "Database connection should not be NULL");

  // Reset tables
  drop_all_tables(database);
  create_tables(database);

  // Create mock users
  user user1 = {
      .userID = 1,
      .name = "User One",
      .OMG = 100,
      .DOGE = 200,
      .BTC = 50,
      .ETH = 75,
  };

  user user2 = {
      .userID = 2,
      .name = "User Two",
      .OMG = 150,
      .DOGE = 250,
      .BTC = 60,
      .ETH = 85,
  };

  // Insert users
  int res = insert_user(database, &user1);
  cr_assert_eq(res, SQLITE_OK, "insert_user for user1 failed: %d", res);

  res = insert_user(database, &user2);
  cr_assert_eq(res, SQLITE_OK, "insert_user for user2 failed: %d", res);

  // Create a mock buy order
  order buy_order = {
      .item = COIN_BTC,
      .buyOrSell = BUY,
      .quantity = 10,
      .unitPrice = 600.50,
      .userID = 1,
  };

  // Insert the buy order
  res = insert_order(database, &buy_order);
  cr_assert_eq(res, SQLITE_OK, "insert_order for buy_order failed: %d", res);

  usleep(500000);
  usleep(500000);

  // Create another mock buy order with a different price
  order another_buy_order = {
      .item = COIN_BTC,
      .buyOrSell = BUY,
      .quantity = 10,
      .unitPrice = 101.00,
      .userID = 1,
  };

  // Insert the second buy order
  res = insert_order(database, &another_buy_order);
  cr_assert_eq(res, SQLITE_OK, "insert_order for another_buy_order failed: %d",
               res);

  // Create a mock search order
  order search_order = {
      .item = COIN_BTC,
      .buyOrSell = SELL,
      .quantity = 10,
      .unitPrice = 90,
      .userID = 2,
  };

  // Call the function being tested
  int matching_order_id = find_matching_buy(database, &search_order);
  cr_assert_neq(matching_order_id, -1, "No matching buy order found.");

  // Verify the matching order ID
  const char* verify_sql =
      "SELECT orderID FROM orders WHERE orderID = ? AND buyOrSell = ?;";
  sqlite3_stmt* stmt = NULL;
  res = sqlite3_prepare_v2(database, verify_sql, -1, &stmt, NULL);
  cr_assert_eq(res, SQLITE_OK, "Failed to prepare verification statement: %s",
               sqlite3_errmsg(database));

  res = sqlite3_bind_int(stmt, 1, matching_order_id);
  cr_assert_eq(res, SQLITE_OK, "Failed to bind order ID: %s",
               sqlite3_errmsg(database));

  res = sqlite3_bind_int(stmt, 2, BUY);
  cr_assert_eq(res, SQLITE_OK, "Failed to bind buyOrSell: %s",
               sqlite3_errmsg(database));

  res = sqlite3_step(stmt);
  cr_assert_eq(res, SQLITE_ROW,
               "Matching buy order not found in the database.");

  dump_database(database);
  printf("Matching order ID: %d\n", matching_order_id);

  sqlite3_finalize(stmt);
  close_database(database);
}
Test(test_orders, test_get_user_all_orders) {
  // Open database
  sqlite3* database = open_database();
  cr_assert_not_null(database, "Database connection should not be NULL");

  // Reset tables
  drop_all_tables(database);
  create_tables(database);

  // Insert a user (required for foreign key constraint)
  user new_user = {
      .userID = 1,
      .name = "Test User",
      .OMG = 100,
      .DOGE = 200,
      .BTC = 50,
      .ETH = 75,
  };

  int res = insert_user(database, &new_user);
  cr_assert_eq(res, SQLITE_OK, "insert_user failed: %d", res);

  // Insert multiple orders for the user
  order order1 = {.item = COIN_BTC,
                  .buyOrSell = BUY,
                  .quantity = 5,
                  .unitPrice = 100.0,
                  .userID = new_user.userID};
  order order2 = {.item = COIN_BTC,
                  .buyOrSell = SELL,
                  .quantity = 3,
                  .unitPrice = 90.0,
                  .userID = new_user.userID};
  order order3 = {.item = COIN_ETH,
                  .buyOrSell = BUY,
                  .quantity = 2,
                  .unitPrice = 200.0,
                  .userID = new_user.userID};

  res = insert_order(database, &order1);
  cr_assert_eq(res, SQLITE_OK, "Failed to insert order1: %d", res);
  res = insert_order(database, &order2);
  cr_assert_eq(res, SQLITE_OK, "Failed to insert order2: %d", res);
  res = insert_order(database, &order3);
  cr_assert_eq(res, SQLITE_OK, "Failed to insert order3: %d", res);

  // Test get_user_all_orders
  order orders[10];  // Allocate space for up to 10 orders
  int order_count = 0;
  res = get_user_all_orders(database, new_user.userID, orders, &order_count);
  cr_assert_eq(res, SQLITE_OK, "get_user_all_orders failed: %d", res);
  cr_assert_eq(order_count, 3, "Expected 3 orders for the user, but got %d",
               order_count);

  // Dump all orders for verification
  printf("Dumping all orders for user ID %d:\n", new_user.userID);
  for (int i = 0; i < order_count; i++) {
    printf(
        "Order %d: orderID=%d, item=%d, buyOrSell=%d, quantity=%d, "
        "unitPrice=%.2f, userID=%d, created_at=%s\n",
        i + 1, orders[i].orderID, orders[i].item, orders[i].buyOrSell,
        orders[i].quantity, orders[i].unitPrice, orders[i].userID,
        orders[i].created_at);
    free(orders[i].created_at);  // Free the allocated created_at string
  }

  close_database(database);
}
