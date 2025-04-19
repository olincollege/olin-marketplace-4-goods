#pragma once
#include <sqlite3.h>

/**
 * @enum TransactionType
 * @brief Represents the type of transaction: BUY or SELL.
 */
typedef enum { BUY = 0, SELL = 1 } TransactionType;

/**
 * @enum CoinType
 * @brief Represents the type of cryptocurrency.
 *
 * COIN_OMG - Represents OMG coin.
 * COIN_DOGE - Represents Dogecoin.
 * COIN_BTC - Represents Bitcoin.
 * COIN_ETH - Represents Ethereum.
 */
typedef enum {
  COIN_OMG = 0,
  COIN_DOGE = 1,
  COIN_BTC = 2,
  COIN_ETH = 3
} CoinType;

/**
 * @struct user
 * @brief Represents a user and their cryptocurrency inventory.
 *
 * @var user::userID
 * User's unique identifier.
 *
 * @var user::name
 * User's name.
 *
 * @var user::OMG
 * Amount of OMG coins owned by the user.
 *
 * @var user::DOGE
 * Amount of Dogecoin owned by the user.
 *
 * @var user::BTC
 * Amount of Bitcoin owned by the user.
 *
 * @var user::ETH
 * Amount of Ethereum owned by the user.
 */
typedef struct {
  int userID;
  char* name;
  int OMG;
  int DOGE;
  int BTC;
  int ETH;
} user;

/**
 * @struct order
 * @brief Represents an order for buying or selling cryptocurrency.
 *
 * @var order::orderID
 * Unique identifier for the order.
 *
 * @var order::item
 * The type of cryptocurrency being traded (refer to CoinType).
 *
 * @var order::buyOrSell
 * Indicates whether the order is a buy (0) or sell (1) transaction.
 *
 * @var order::quantity
 * The quantity of cryptocurrency being traded.
 *
 * @var order::userID
 * The ID of the user who placed the order.
 *
 * @var order::unitPrice
 * The price per unit of the cryptocurrency.
 *
 * @var order::created_at
 * Timestamp indicating when the order was created.
 */
typedef struct {
  int orderID;
  int item;
  int buyOrSell;  // 0 for buy, 1 for sell
  int quantity;
  int userID;
  double unitPrice;
  char* created_at;  // Timestamp for when the order was created
} order;

/**
 * @def database_FILENAME
 * @brief Default filename for the SQLite database.
 */
#define FILENAME "database.db"

/**
 * @brief Opens a SQLite3 database for use.
 *
 * @return SQLITE_OK on success, or an error code on failure.
 */
sqlite3* open_database(void);

/**
 * @brief Closes the SQLite3 database.
 *
 * @param database A pointer to the SQLite3 database to close.
 * @return SQLITE_OK on success, or an error code on failure.
 */
int close_database(sqlite3* database);

/**
 * @brief Creates the required tables for the database.
 *
 * This function creates the following tables if they do not already exist:
 * - users: Stores user information and their cryptocurrency inventory.
 * - orders: Stores active orders for buying or selling cryptocurrency.
 * - archives: Stores archived orders for historical purposes.
 *
 * @param database A pointer to the SQLite3 database.
 * @return SQLITE_OK on success, or an error code on failure.
 */
int create_tables(sqlite3* database);

/**
 * @brief Inserts an item record into the inventory table.
 *
 * @param database A pointer to the SQLite3 database.
 * @param id The ID of the item.
 * @param omg The OMG value of the item.
 * @param coin1 The first coin value of the item.
 * @param coin2 The second coin value of the item.
 * @param coin3 The third coin value of the item.
 * @return SQLITE_OK on success, or an error code on failure.
 */
int insert_item(sqlite3* database, int id, int omg, int coin1, int coin2,
                int coin3);

/**
 * @brief Selects and prints all records from the inventory table.
 *
 * @param database A pointer to the SQLite3 database.
 * @return SQLITE_OK on success, or an error code on failure.
 */
int select_all(sqlite3* database);

/**
 * @brief Updates an item record in the inventory table.
 *
 * @param database A pointer to the SQLite3 database.
 * @param id The ID of the item to update.
 * @param omg The new OMG value of the item.
 * @param coin1 The new first coin value of the item.
 * @param coin2 The new second coin value of the item.
 * @param coin3 The new third coin value of the item.
 * @return SQLITE_OK on success, or an error code on failure.
 */
int update_item(sqlite3* database, int id, int omg, int coin1, int coin2,
                int coin3);

/**
 * @brief Deletes an item record from the inventory table.
 *
 * @param database A pointer to the SQLite3 database.
 * @param id The ID of the item to delete.
 * @return SQLITE_OK on success, or an error code on failure.
 */
int delete_item(sqlite3* database, int id);

/**
 * @brief Inserts an order record into the orders table.
 *
 * @param database A pointer to the SQLite3 database.
 * @param new_order A pointer to the order structure containing order details.
 * @return SQLITE_OK on success, or an error code on failure.
 */
int insert_order(sqlite3* database, order* new_order);

/**
 * @brief Selects orders for a specific user and populates an order list.
 *
 * @param database A pointer to the SQLite3 database.
 * @param userID The ID of the user whose orders are to be selected.
 * @param orderList A pointer to an array of order structures to store the
 * results.
 * @param orderCount A pointer to an integer to store the number of orders
 * retrieved.
 * @return SQLITE_OK on success, or an error code on failure.
 */
int select_user_orders(sqlite3* database, int userID, order* orderList,
                       int* orderCount);
