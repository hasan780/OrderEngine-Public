cmake_minimum_required(VERSION 3.20)
project(order_engine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ----- Library -----
add_library(order_engine_lib
    src/order_book.cpp
    src/matching_engine.cpp
)
target_include_directories(order_engine_lib PUBLIC include)

# ----- Tests (requires Catch2 installed via vcpkg or system package) -----
find_package(Catch2 3 REQUIRED)

add_executable(tests
    tests/test_order_book.cpp
    tests/test_matching_engine.cpp
)
target_link_libraries(tests PRIVATE order_engine_lib Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(tests)
