#pragma once

#include <netinet/in.h>
#include <pthread.h>
#include <sqlite3.h>
#include <stdio.h>
#include <sys/socket.h>

enum { BACKLOG_SIZE = 10 };

// Group the data needed for a server to run.
typedef struct {
  /// The socket descriptor to listen for connections.
  int listener;
  /// The address and port for the listener socket.
  struct sockaddr_in addr;
  /// The maximum number of clients that can be waiting to connect at once.
  int max_backlog;
} echo_server;

/**
 * Create a new echo server in dynamic memory.
 *
 * Given a socket address and a maximum backlog size, create a new echo server
 * on the heap. Since the new server (or rather, the data it stores) is
 * dynamically allocated, the caller is responsible for cleaning the server up
 * afterwards (or terminating the program and letting that take care of things).
 *
 * @param addr The IPv4 address and port that the server will listen on.
 * @param max_backlog The max number of clients that can wait to connect to the
 * server.
 * @return A pointer to the new echo server.
 */
echo_server* make_echo_server(struct sockaddr_in ip_addr, int max_backlog);

/**
 * Free an echo server in dynamic memory.
 *
 * Given a pointer to an echo server on the heap, free the dynamically allocated
 * memory associated with that server. Attempting to free an uninitialized
 * server, server dynamic memory that has already been freed, or a pointer to
 * memory representing anything other than an echo_server instance will result
 * in undefined behavior.
 *
 * @param server A pointer to the server to delete.
 */
void free_echo_server(echo_server* server);

/**
 * Start listening on a server for potential client connections.
 *
 * Bind the listener socket and set it to listen for client connection requests.
 * The listener socket is part of the server's internal data and does not need
 * to be opened separately. In the event that binding or listening on the socket
 * fails, print an error message and exit the program without returning.
 *
 * @param server The server to start listening on.
 */
void listen_for_connections(echo_server* server);

/**
 * Accept a new client connection and start the echo process.
 *
 * Wait until a client connection request comes in, accepting it. Once accepted,
 * fork a new process. In the child process, run the echo process, and in the
 * parent, end the function. The server program should run the function in a
 * loop, whereas the child can exit the function after the echo process. In the
 * event of an error in accepting a connection or forking a new process, print
 * an appropriate error message and accept the program.
 *
 * @param server The server to accept the connection on.
 * @return 0 for the parent process and -1 for the child (echo) process.
 */
int accept_client(echo_server* server, sqlite3* database);

/**
 * @brief Registers a new user by interacting with the client through a
 * communication file and storing the user's information in the database.
 *
 * This function prompts the user for a username, display name, and password via
 * the provided communication file. It then validates and processes the input,
 * and registers the user in the database with default cryptocurrency balances.
 * If successful, the function returns the newly created user's ID.
 *
 * @param comm_file A pointer to a FILE object used for communication with the
 * client. This is typically a socket or file stream for sending and receiving
 * data.
 * @param database  A pointer to an SQLite3 database connection where the user
 * information will be stored.
 *
 * @return The ID of the newly registered user on success, or -1 if an error
 * occurs during the registration process.
 *
 * @note The function will terminate the program if it encounters a critical
 * error, such as being unable to send prompts or read input from the
 * communication file.
 *
 * @warning The function assumes that the communication file and database
 * connection are valid and properly initialized. It also assumes that the
 * database schema supports the `insert_user` function for adding new users.
 */
int register_user(FILE* comm_file, sqlite3* database);

/**
 * Handle client communication by echoing messages and interacting with the
 * database.
 *
 * This function manages communication with a connected client through a socket.
 * It reads messages from the client, processes them, and sends appropriate
 * responses back to the client. The function also interacts with a SQLite
 * database to perform operations based on the client's requests. The
 * communication continues until the client disconnects or an error occurs.
 *
 * @param comm_file A file stream associated with the socket connection to the
 * client.
 * @param userID The unique identifier of the authenticated user.
 * @param database A pointer to the SQLite database connection for handling
 * client requests.
 *
 * @note The function ensures proper cleanup of resources, such as closing the
 * file stream, before returning or exiting. Critical errors, such as socket
 * communication failures or database access issues, will result in program
 * termination.
 */
void echo(FILE* comm_file, int userID, sqlite3* database);

/**
 * Handles user authentication by communicating over a socket connection.
 *
 * This function facilitates user authentication by interacting with a client
 * through a socket. It requests a username and password from the client and
 * verifies the credentials against a SQLite database. If the credentials are
 * valid, the user is authenticated, and a success message is sent back to the
 * client. If the credentials are invalid, the client is prompted to retry.
 *
 * @param comm_file A file stream associated with the socket connection to the
 * client.
 * @param database A pointer to the SQLite database connection used for
 * verifying user credentials.
 * @return Returns 0 (EXIT_SUCCESS) if authentication is successful. The
 * function terminates the program on critical errors.
 *
 * @note The function ensures that any dynamically allocated memory for the
 * username and password is properly freed before returning or exiting.
 * @note Critical errors, such as socket communication failures or database
 * access issues, will result in program termination.
 */
int authenticate(FILE* comm_file, sqlite3* database);
