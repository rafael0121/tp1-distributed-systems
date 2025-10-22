#
# Dependencies
#
find_package(Threads)

#
# Sources
#
set(SOURCES
    "Source Files/main.cpp"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})


#
# Target
#
add_executable(server ${SOURCES})
target_link_libraries(server PRIVATE myproto)
target_link_libraries(server PRIVATE lamport)
