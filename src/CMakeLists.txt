find_package(SQLite3 REQUIRED)
include_directories(${SQLite3_INCLUDE_DIRS})

add_library(util util.c util.h)

add_library(client client.c client.h)
target_link_libraries(client PRIVATE util)

add_executable(run_client run_client.c)
target_link_libraries(run_client PRIVATE client util)

add_library(server server.c server.h)
target_link_libraries(server PRIVATE util)

add_executable(run_server run_server.c)
target_link_libraries(run_server PRIVATE server util)

add_library(db db.c db.h)
target_link_libraries(db PRIVATE util ${SQLite3_LIBRARIES})  # <-- Link sqlite3 here

add_library(command command.c command.h)
target_link_libraries(command PRIVATE util db)
