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
 * @brief Views the details of an order by item ID.
 *
 * @param itemID The ID of the item to view.
 * @return Pointer to the order struct containing the order details, or NULL if
 * not found.
 */
order* view(int itemID);
