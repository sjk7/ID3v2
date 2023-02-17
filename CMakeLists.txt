cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)
project(ID3Tags LANGUAGES CXX)

# warning levels, JustMyCode, Edit and Continue:
include(${CMAKE_CURRENT_SOURCE_DIR}../config/CMakeLists.txt)


enable_testing()
add_executable(ID3HeaderApp "src/main.cpp" include/myUtils.hpp include/ID3v2.hpp include/FileDataReader.hpp include/IDataReader.hpp)

#install(TARGETS ID3HeaderApp
#    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

add_executable (tddfile Tests/tddfile.cpp include/myUtils.hpp)

add_executable(tddFileReader Tests/tddFileReader.cpp include/FileDataReader.hpp
    include/IDataReader.hpp)

add_executable(BadID3HeaderTest Tests/tddID3Header.cpp include/myUtils.hpp include/ID3v2.hpp
include/FileDataReader.hpp include/IDataReader.hpp)

add_executable(ReadValidTagHeader Tests/tddReadValidTagHeader.cpp include/ID3v2.hpp)
add_executable(TestID3Skipper Tests/tddID3Skipper.cpp)

add_test(NAME TestFile COMMAND tddfile)
add_test(NAME FileReader COMMAND tddFileReader)
add_test(NAME TestBadHeader COMMAND BadID3HeaderTest)
add_test(NAME ReadValidHeaderTag COMMAND ReadValidTagHeader)
add_test(NAME ID3Skipper COMMAND TestID3Skipper)

# tddfile, tddFileReader, BadID3Headertest, ReadValidTagHeader, TestID3Skipper)
#
#add_custom_command(
#     TARGET ${UNIT_TEST}
#     COMMENT "Run tests"
#     POST_BUILD 
#     COMMAND ${UNIT_TEST}
#)
#






