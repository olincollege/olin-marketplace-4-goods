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
 * @brief Inserts a new order into the "orders" table in the SQLite database.
 *
 * This function constructs an SQL INSERT statement using the details of the
 * provided `order` structure and executes it on the given SQLite database.
 *
 * @param database A pointer to the SQLite database connection.
 * @param new_order A pointer to the `order` structure containing the details
 *                  of the order to be inserted. The structure should include:
 *                  - item: The item ID.
 *                  - buyOrSell: Indicator of buy (e.g., 1) or sell (e.g., 0).
 *                  - quantity: The quantity of the item.
 *                  - unitPrice: The price per unit of the item.
 *                  - userID: The ID of the user placing the order.
 *
 * @return Returns `SQLITE_OK` (0) on success. On failure, it returns an SQLite
 *         error code and frees any allocated error message.
 */
int insert_order(sqlite3* database, order* new_order);

/**
 * @brief Drops all tables from the given SQLite database.
 *
 * This function disables foreign key constraints temporarily, begins a
 * transaction, and drops the `users`, `orders`, and `archives` tables if they
 * exist. After the operation, it re-enables foreign key constraints.
 *
 * @param database A pointer to the SQLite database connection.
 * @return SQLITE_OK on success, or SQLITE_ERROR if an error occurs during the
 * operation.
 *
 * @note This function assumes the database connection is valid and open.
 *       Ensure that no other operations are being performed on the database
 *       while this function is executed.
 * @warning Dropping tables is a destructive operation. Ensure that this
 * function is called only when the data in the tables is no longer needed.
 */
int drop_all_tables(sqlite3* database);
