#
# Sources
#
set(SOURCES
    src/lamport.cpp
)

# Create a static library from source files
add_library(lamport STATIC ${SOURCES})

# Optionally specify include directories
target_include_directories(lamport PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

