#include <arpa/inet.h>  // sockaddr_in
#include <pthread.h>    // pthread_mutex_t and related functions
#include <sqlite3.h>
#include <stddef.h>  // For NULL
#include <sys/mman.h>

#include "command.h"
#include "server.h"  // echo_server, related functions
#include "util.h"    // socket_address, PORT

int main(void) {
  // Spin up database
  sqlite3* db_ptr = NULL;
  if (open_db(&db_ptr) == -1) {
    error_and_exit("Can't open databse!");
  }
  if (init_db(db_ptr) == -1) {
    error_and_exit("Can't initialize database!");
  }

  // Get userID
  int userID = 1;
  user new_user = {
      .userID = 1,
      .name = "Test User",
      .OMG = 100,
      .DOGE = 200,
      .BTC = 50,
      .ETH = 75,
  };
  int res = insert_user(db_ptr, &new_user);

  pthread_mutex_t* mutex;
  mutex = mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE,
               MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (mutex == MAP_FAILED) {
    error_and_exit("Can't create mutex");
  }

  // Initialize the mutex for process sharing
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
  pthread_mutex_init(mutex, &attr);
  pthread_mutexattr_destroy(&attr);

  struct sockaddr_in server_addr = socket_address(INADDR_ANY, PORT);
  echo_server* server = make_echo_server(server_addr, BACKLOG_SIZE);
  listen_for_connections(server);
  int accept_status = 0;
  while (accept_status != -1) {
    accept_status = accept_client(server, mutex, userID, db_ptr);
  }
  free_echo_server(server);
  return 0;

  // Need to check for parent process
  if (close_db(&db_ptr) == -1) {
    error_and_exit("Can't close database!");
  }
}
