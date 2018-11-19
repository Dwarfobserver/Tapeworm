
project(tapeworm)
cmake_minimum_required(VERSION 3.11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
enable_testing()

add_library(tapeworm INTERFACE)
target_include_directories(tapeworm INTERFACE
    "${CMAKE_SOURCE_DIR}/include")

add_executable(tests
    "${CMAKE_SOURCE_DIR}/tests/main.cpp"
    "${CMAKE_SOURCE_DIR}/tests/concepts.cpp"
    "${CMAKE_SOURCE_DIR}/tests/serialization.cpp")
target_link_libraries(tests PRIVATE tapeworm)
add_test(NAME tests COMMAND tests)

if (MSVC)
    add_compile_options(tests PRIVATE "/W4 /WX")
else ()
    add_compile_options(tests PRIVATE "-Wall" "-Wextra" "-Werror")
endif()
