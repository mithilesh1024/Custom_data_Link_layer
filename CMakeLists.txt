cmake_minimum_required(VERSION 3.16)

project(DataLinkLayerProject VERSION 1.0 LANGUAGES CXX)

# Use modern C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find pthreads properly
find_package(Threads REQUIRED)

# ---- Library (common code) ----
add_library(datalink_lib
    src/SerialPort.cpp
    src/Frame.cpp
    src/DataLinkLayer.cpp
)

target_include_directories(datalink_lib PUBLIC include)

# Link pthread to the library
target_link_libraries(datalink_lib PUBLIC Threads::Threads)

# ---- Communicator A ----
add_executable(communicatorA
    apps/communicatorA.cpp
)

target_link_libraries(communicatorA
    datalink_lib
)

# ---- Communicator B ----
add_executable(communicatorB
    apps/communicatorB.cpp
)

target_link_libraries(communicatorB
    datalink_lib
)

# ---- Add compile-time flag for both executables ----
target_compile_definitions(communicatorA PUBLIC COMM_TEST_MODE=0)
target_compile_definitions(communicatorB PUBLIC COMM_TEST_MODE=0)