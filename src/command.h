#pragma once

#include "db.h"
#include "util.h"

/**
 * @brief Opens a SQLite database connection.
 *
 * This function initializes a SQLite database connection and assigns it to
 * the provided pointer. If the database cannot be opened, it returns an error
 * code.
 *
 * @param[out] database A pointer to a pointer of type `sqlite3` where the
 * database connection will be stored.
 * @return int Returns 0 on success, or -1 if the database connection could not
 * be opened.
 */
int open_db(sqlite3** database);

/**
 * @brief Initializes the SQLite database by creating necessary tables.
 *
 * This function creates the required tables in the provided SQLite database.
 * If table creation fails, it attempts to close the database and returns an
 * error code.
 *
 * @param[in] database A pointer to an open SQLite database connection.
 * @return int Returns 0 on success, or -1 if table creation fails or if the
 * database could not be closed properly after a failure.
 */
int init_db(sqlite3* database);

/**
 * @brief Closes the database connection.
 *
 * @param database Pointer to the SQLite database connection.
 * @return 0 on success, or exits the program on failure.
 */
int close_db(sqlite3* database);

/**
 * @brief Creates a new order from a string array of parameters.
 *
 * This function allocates memory for a new order and initializes its fields
 * based on the provided string array of parameters and the user ID. It
 * parses the string values for unit price and quantity, determines whether
 * the order is a buy or sell order, and assigns the user ID. If memory
 * allocation fails, it returns NULL. If parsing fails for unit price or
 * quantity, a warning is printed to the console.
 *
 * @param params A pointer to a string_array containing the order
 * parameters.
 *               - params->strings[0]: "buy" or "sell" to indicate the order
 * type.
 *               - params->strings[2]: The unit price as a string.
 *               - params->strings[3]: The quantity as a string.
 * @param userID The identifier of the user creating the order.
 * @return A pointer to the newly created order, or NULL if memory
 * allocation fails.
 */
order* create_order_from_string(string_array* params, int userID);

/**
 * @brief Creates a new order with the specified parameters.
 *
 * This function allocates memory for a new order and initializes its fields
 * with the provided values. If memory allocation fails, it returns NULL.
 *
 * @param item The item identifier for the order.
 * @param buyOrSell Indicates whether the order is a buy (1) or sell (0) order.
 * @param quantity The quantity of the item in the order.
 * @param unitPrice The price per unit of the item.
 * @param userID The identifier of the user creating the order.
 * @return A pointer to the newly created order, or NULL if memory allocation
 * fails.
 */
order* create_order(int item, int buyOrSell, int quantity, double unitPrice,
                    int userID);

/**
 * @brief Frees the memory allocated for an order.
 *
 * This function deallocates the memory associated with the given order. If
 * the provided order pointer is NULL, it returns -1 to indicate an error.
 * Otherwise, it frees the memory and returns 0 to indicate success.
 *
 * @param ord A pointer to the order to be freed.
 * @return 0 if the order was successfully freed, or -1 if the order pointer
 * is NULL.
 */
int free_order(order* ord);

/**
 * @brief Allocates and initializes a new user struct.
 *
 * This function creates a user struct on the heap and sets its fields using
 * the provided parameters. It also allocates memory for the user's name.
 *
 * @param userID The user's unique ID.
 * @param name The user's name (will be copied internally).
 * @param OMG The amount of OMG the user has.
 * @param DOGE The amount of DOGE the user has.
 * @param BTC The amount of BTC the user has.
 * @param ETH The amount of ETH the user has.
 * @return A pointer to the created user struct, or NULL if allocation fails.
 */
user* create_user(int userID, const char* name, int OMG, int DOGE, int BTC,
                  int ETH);  // Defined below

/**
 * @brief Frees the memory associated with a user struct.
 *
 * This function releases the dynamically allocated memory for both the user's
 * name and the user struct itself.
 *
 * @param usr Pointer to the user struct to be freed.
 * @return 0 on success, -1 if the user pointer is NULL.
 */
int free_user(user* usr);  // Defined below

/**
 * @brief Retrieves the inventory of a user.
 *
 * @param userID The ID of the user.
 * @param cur_user Pointer to a user struct to store the retrieved user data.
 * @return 0 on success, or exits the program on failure.
 */
int myInventory(int userID, user** cur_user);

/**
 * @brief Places a buy order in the database.
 *
 * @param database Pointer to the SQLite database connection.
 * @param ord Pointer to the order struct containing order details.
 * @return 0 on success, -1 on failure.
 */
int buy(sqlite3* database, order* ord);

/**
 * @brief Places a sell order in the database.
 *
 * @param database Pointer to the SQLite database connection.
 * @param ord Pointer to the order struct containing order details.
 * @return 0 on success, -1 on failure.
 */
int sell(sqlite3* database, order* ord);

/**
 * @brief Retrieves all orders placed by a user.
 *
 * @param database Pointer to the SQLite database connection.
 * @param userID The ID of the user.
 * @param orderList Pointer to an array of order structs to store the retrieved
 * orders.
 * @param orderCount Pointer to an integer to store the number of retrieved
 * orders.
 */
void myOrders(sqlite3* database, int userID, order** orderList,
              int* orderCount);

/**
 * @brief Retrieves and displays all buy and sell orders for a specific item.
 *
 * @param database Pointer to the SQLite database connection.
 * @param itemID The ID of the item for which orders are to be retrieved.
 * @param buy_orders Pointer to an array of buy order structs to be populated.
 * @param buy_count Pointer to an integer to store the count of buy orders.
 * @param sell_orders Pointer to an array of sell order structs to be populated.
 * @param sell_count Pointer to an integer to store the count of sell orders.
 */
void viewItemOrders(sqlite3* database, int itemID, order** buy_orders,
                    int* buy_count, order** sell_orders, int* sell_count);

/**
 * @brief Retrieves the inventory of a specific user from the database.
 *
 * This function fetches the inventory associated with the given user by
 * delegating the operation to the `get_user_inventories` function.
 *
 * @param database A pointer to the SQLite3 database connection.
 * @param usr A pointer to the user structure representing the user whose
 *            inventory is to be retrieved.
 * @return An integer indicating the result of the operation. The exact
 *         meaning of the return value depends on the implementation of
 *         `get_user_inventories`.
 */
int get_user_inventory(sqlite3* database, user* usr);
