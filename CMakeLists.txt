cmake_minimum_required(VERSION 3.16)

project(DataLinkLayerProject VERSION 1.0 LANGUAGES CXX)

# Use modern C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include headers
include_directories(include)

# ---- Library (common code) ----
add_library(datalink_lib
    src/SerialPort.cpp
    src/Frame.cpp
    src/DataLinkLayer.cpp
)

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