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

typedef enum {
  DEFAULT_OMG = 10000,
  DEFAULT_DOGE = 200,
  DEFAULT_BTC = 50,
  DEFAULT_ETH = 75
} CoinAmount;

/**
 * @struct user
 * @brief Represents a user and their cryptocurrency inventory.
 *
 * @var user::userID
 * User's unique identifier.
 *
 * @var user::username
 * User's login username (must be unique).
 *
 * @var user::password
 * User's login password (stored as plaintext for now).
 *
 * @var user::name
 * User's display name.
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
  char* username;
  char* password;
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
 * @def BUSY_TIMEOUT
 * @brief Default busy timeout for SQLite operations in milliseconds.
 */
#define BUSY_TIMEOUT 1000

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

/**
 * @brief Inserts a new order into the "orders" table in the SQLite database.
 *
 * This function prepares an SQL INSERT statement to add a new order to the
 * "orders" table. It binds the values from the provided `order` structure
 * to the SQL statement and executes it.
 *
 * @param database A pointer to the SQLite database connection.
 * @param new_order A pointer to the `order` structure containing the details
 *                  of the order to be inserted. The structure should include:
 *                  - item: The type of cryptocurrency being traded.
 *                  - buyOrSell: Indicator of buy (0) or sell (1).
 *                  - quantity: The quantity of the cryptocurrency.
 *                  - unitPrice: The price per unit of the cryptocurrency.
 *                  - userID: The ID of the user placing the order.
 *
 * @return Returns `SQLITE_OK` (0) on success. On failure, it returns an SQLite
 *         error code and logs the error message to stderr.
 *
 * @note Ensure that the database connection is valid and open before calling
 *       this function.
 * @warning This function does not validate the input data. Ensure that the
 *          `new_order` structure contains valid values.
 */
int insert_order(sqlite3* database, order* new_order);

/**
 * Inserts a new user record into the "users" table in the SQLite database.
 *
 * @param database A pointer to the SQLite database connection.
 * @param new_user A pointer to a user structure containing the data to be
 * inserted.
 * @param user_ID A pointer to an integer where the newly inserted user's ID
 * will be stored. This value is set to the row ID of the inserted record.
 * @return SQLITE_OK on success, or an SQLite error code on failure.
 *
 * This function prepares an SQL INSERT statement, binds the user data to the
 * statement, executes it, and finalizes the statement. If any step fails, an
 * error message is printed to stderr and the corresponding SQLite error code
 * is returned.
 */
int insert_user(sqlite3* database, user* new_user, int* user_ID);

/**
 * @brief Prints the contents of specific tables from a SQLite database.
 *
 * Iterates through predefined SQL queries for "users", "orders", and "archives"
 * tables, executes them, and displays the results in a tabular format.
 *
 * @param database Pointer to an open SQLite database connection.
 *
 * Handles:
 * - INTEGER, FLOAT, TEXT, and NULL data types.
 * - Errors during query preparation or execution.
 *
 * Assumes:
 * - The database connection is valid and tables exist.
 */
void dump_database(sqlite3* database);

/**
 * Deletes an order by ID from the "orders" table.
 */
int delete_order(sqlite3* database, int orderID);

/**
 * @brief Retrieves a user by userID.
 *
 * Allocates a copy of the username string on the heap.
 * Caller is responsible for freeing the memory.
 *
 * @param database A pointer to the SQLite database connection.
 * @param userID The ID of the user to retrieve.
 * @param user_out A pointer to a user struct to populate.
 * @return SQLITE_OK if found, SQLITE_NOTFOUND if not, or another SQLite error
 * code.
 */
int get_user(sqlite3* database, int userID, user* user_out);

/**
 * Finds a matching buy order in the database for the given search order.
 *
 * This function searches for a buy order in the "orders" table that matches
 * the specified criteria:
 * - The item matches the item in the search order.
 * - The order is a buy order (buyOrSell = 0).
 * - The unit price is greater than or equal to the unit price in the search
 * order.
 * - The user ID is different from the user ID in the search order.
 *
 * The matching order is selected based on the earliest creation time
 * (ordered by `created_at` in ascending order) and is limited to one result.
 *
 * @param database A pointer to the SQLite database connection.
 * @param search_order A pointer to the `order` structure containing the search
 * criteria.
 * @return The `orderID` of the matching buy order if found, or -1 if no
 * matching order is found or if an error occurs during the query.
 */
int find_matching_buy(sqlite3* database, order* search_order);

/**
 * @brief Finds a matching sell order in the database for the given search
 * order.
 *
 * This function queries the database to find a sell order that matches the
 * specified criteria in the `search_order`. The matching sell order must:
 * - Have the same item as the `search_order`.
 * - Be a sell order (buyOrSell = 1).
 * - Have a unit price less than or equal to the unit price of the
 * `search_order`.
 * - Belong to a different user (userID != search_order->userID).
 *
 * The function returns the `orderID` of the first matching sell order, ordered
 * by creation time in ascending order. If no matching order is found, it
 * returns -1.
 *
 * @param database A pointer to the SQLite database connection.
 * @param search_order A pointer to the `order` structure containing the search
 * criteria.
 * @return The `orderID` of the matching sell order if found, or -1 if no match
 * is found.
 */

int find_matching_sell(sqlite3* database, order* search_order);

/*
 * and must be freed by the caller.
 * @return SQLITE_OK if the order is successfully retrieved, SQLITE_NOTFOUND if
 * the order is not found, or an SQLite error code if an error occurs during the
 * retrieving process.
 *
 * This function prepares and executes an SQL query to fetch the order details
 * from the `orders` table. If the order is found, its details are populated
 * into the `order_out` structure. If the order is not found or an error occurs,
 * appropriate error messages are printed to `stderr`.
 */
int get_order(sqlite3* database, int orderID, order* order_out);

/**
 * Retrieves the top 5 buy and sell orders for a specific item.
 *
 * @param database A pointer to the SQLite database connection.
 * @param item The CoinType (e.g., COIN_BTC, COIN_ETH).
 * @param buy_orders_out Pointer to an array of buy orders. Memory is allocated
 *                       and must be freed by the caller.
 * @param buy_count_out Pointer to an integer to receive the number of buy
 * orders.
 * @param sell_orders_out Pointer to an array of sell orders. Memory is
 * allocated and must be freed by the caller.
 * @param sell_count_out Pointer to an integer to receive the number of sell
 * orders.
 * @return SQLITE_OK on success, or an SQLite error code on failure.
 */
int get_item_all_orders(sqlite3* database, int item, order** buy_orders_out,
                        int* buy_count_out, order** sell_orders_out,
                        int* sell_count_out);

/**
 * Updates an existing order in the "orders" table.
 *
 * @param database A pointer to the SQLite database connection.
 * @param updated_order Pointer to the updated order struct (must have orderID
 * filled).
 * @return SQLITE_OK when it is a success, or an SQLite error code when it is a
 * failure.
 */
int update_order(sqlite3* database, const order* updated_order);

/**
 * Updates the coin balances of a user that is already a part of the database.
 *
 * @param database A pointer to the SQLite database connection.
 * @param updated_user Pointer to the user struct with updated balances (userID
 * should be given).
 * @return SQLITE_OK on success, or an SQLite error code on failure.
 */
int update_user_balance(sqlite3* database, const user* updated_user);

/**
 * Retrieves all orders associated with a specific user from the database.
 *
 * @param database A pointer to the SQLite3 database connection.
 * @param userID The ID of the user whose orders are to be retrieved.
 * @param orders_out A pointer to an array of order structures where the
 * retrieved orders will be stored. The caller is responsible for freeing the
 * allocated memory.
 * @param count_out A pointer to an integer where the number of retrieved orders
 * will be stored.
 *
 * @return An integer indicating the success or failure of the operation.
 *         Returns 0 on success, or a non-zero error code on failure.
 */

int get_user_all_orders(sqlite3* database, int userID, order** orders_out,
                        int* count_out);

/**
 * @brief Retrieves the cryptocurrency inventory for a specific user from the
 * database.
 *
 * This function queries the database to fetch the inventory of cryptocurrencies
 * (OMG, DOGE, BTC, ETH) for a user identified by their userID. The results are
 * stored in the provided `user` structure.
 *
 * @param database A pointer to the SQLite database connection.
 * @param user_out A pointer to a `user` structure where the retrieved inventory
 *                 will be stored. The `userID` field of this structure must be
 *                 set prior to calling this function.
 *
 * @return SQLITE_OK if the user's inventory is successfully retrieved.
 *         SQLITE_NOTFOUND if the user with the specified userID is not found.
 *         An SQLite error code if there is an issue with preparing or executing
 *         the SQL statement.
 *
 * @note The function assumes that the `user` structure has fields named OMG,
 *       DOGE, BTC, and ETH to store the respective cryptocurrency values.
 * @note The caller is responsible for ensuring that the database connection is
 *       valid and that the `user_out` pointer is not NULL.
 */

int get_user_inventories(sqlite3* database, user* user_out);

/**
 * Retrieves a user record from the database by username.
 *
 * This function queries the `users` table in the provided SQLite database
 * to find a user with the specified username. If a matching user is found,
 * their details are populated into the provided `user_out` structure.
 *
 * @param database A pointer to the SQLite database connection.
 * @param username The username of the user to retrieve.
 * @param user_out A pointer to a `user` structure where the retrieved user
 *                 information will be stored. The caller is responsible for
 *                 freeing any dynamically allocated strings in this structure.
 *
 * @return `SQLITE_OK` if the user is successfully retrieved and `user_out` is
 * populated. `SQLITE_NOTFOUND` if no user with the specified username is found.
 *         An SQLite error code if an error occurs during the query execution.
 *
 * @note The function dynamically allocates memory for the `username`,
 * `password`, and `name` fields in the `user_out` structure. The caller must
 * free these fields to avoid memory leaks.
 *
 * @note The function logs errors to `stderr` if any SQLite operation fails.
 */

int get_user_by_username(sqlite3* database, const char* username,
                         user* user_out);

/**
 * Inserts an archived order into the "archives" table in the SQLite database.
 *
 * @param database A pointer to the SQLite database connection.
 * @param archived_order A pointer to the order structure containing the data to
 * be archived.
 * @return SQLITE_OK on success, or an SQLite error code on failure.
 *
 * This function prepares an SQL INSERT statement to add a new record to the
 * "archives" table. It binds the fields of the `archived_order` structure to
 * the appropriate placeholders in the SQL statement and executes the statement.
 * If the operation fails at any step, an error message is printed to `stderr`,
 * and the corresponding SQLite error code is returned.
 */
int insert_archive(sqlite3* database, const order* archived_order);

/**
 * Retrieves all archived orders for a specific user from the "archives" table
 * in the SQLite database.
 *
 * @param database A pointer to the SQLite database connection.
 * @param userID The ID of the user whose archived orders are to be retrieved.
 * @param orders_out A pointer to a dynamically allocated array of `order`
 * structures to store the results.
 * @param count_out A pointer to an integer to store the number of retrieved
 * orders.
 * @return SQLITE_OK on success, or an SQLite error code on failure.
 *
 * This function prepares an SQL SELECT statement to query the "archives" table
 * for orders associated with the specified `userID`. It dynamically allocates
 * memory to store the results and populates the `orders_out` array with the
 * retrieved data. If memory allocation or any SQLite operation fails, an error
 * message is printed to `stderr`, and the corresponding SQLite error code is
 * returned.
 */
int get_user_archived_orders(sqlite3* database, int userID, order** orders_out,
                             int* count_out);

/**
 * Assigns a timestamp to the specified order and updates it in the database.
 *
 * This function retrieves the current timestamp, assigns it to the provided
 * order, and updates the corresponding record in the SQLite database.
 *
 * @param database A pointer to the SQLite database connection.
 * @param order_to_update A pointer to the order structure that needs to be
 * updated with the current timestamp.
 * @return An integer indicating the success or failure of the operation:
 *         - 0 on success.
 *         - A non-zero error code on failure.
 */
int assign_order_timestamp(sqlite3* database, order* order_to_update);
