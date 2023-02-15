cmake_minimum_required(VERSION 3.5)

project(ID3Tags LANGUAGES CXX)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ID3Tags main.cpp myUtils.hpp)

install(TARGETS ID3Tags
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

add_executable (tddfile tddfile.cpp)

add_test(NAME TestFile COMMAND tddfile)

