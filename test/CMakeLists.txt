add_executable(server server.c)
target_link_libraries(server pnet)
target_include_directories(server PUBLIC ../include)

add_executable(client client.c)
target_link_libraries(client pnet)
target_include_directories(client PUBLIC ../include)