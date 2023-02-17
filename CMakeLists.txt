cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)
project(ID3Tags LANGUAGES CXX)

# 'Just My Code' in VS
if( CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "19.15" )
    add_compile_options( /JMC)
endif()

enable_testing()
add_executable(ID3HeaderApp src/main.cpp include/myUtils.hpp include/ID3v2.hpp include/FileDataReader.hpp include/IDataReader.hpp)

# Edit and Continue in VS
if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "19.15")
  target_compile_options(ID3HeaderApp PUBLIC "/ZI")
   target_link_options(ID3HeaderApp PUBLIC "/INCREMENTAL")
endif()

#install(TARGETS ID3HeaderApp
#    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

add_executable (tddfile Tests/tddfile.cpp include/myUtils.hpp)

add_executable(tddFileReader Tests/tddFileReader.cpp include/FileDataReader.hpp
    include/IDataReader.hpp)

add_executable(BadID3HeaderTest Tests/tddID3Header.cpp include/myUtils.hpp include/ID3v2.hpp
include/FileDataReader.hpp include/IDataReader.hpp)

add_executable(ReadValidTagHeader Tests/tddReadValidTagHeader.cpp include/ID3v2.hpp)

add_test(NAME TestFile COMMAND tddfile)
add_test(NAME FileReader COMMAND tddFileReader)
add_test(NAME TestBadHeader COMMAND BadID3HeaderTest)
add_test(NAME ReadValidHeaderTag COMMAND ReadValidTagHeader)

set(UNIT_TEST tddfile)
add_custom_command(
     TARGET ${UNIT_TEST}
     COMMENT "Run tests"
     POST_BUILD 
     COMMAND ${UNIT_TEST}
)

set(UNIT_TEST tddFileReader)
add_custom_command(
     TARGET ${UNIT_TEST}
     COMMENT "Run tests"
     POST_BUILD 
     COMMAND ${UNIT_TEST}
)

set(UNIT_TEST BadID3HeaderTest)
add_custom_command(
     TARGET ${UNIT_TEST}
     COMMENT "Run tests"
     POST_BUILD 
     COMMAND ${UNIT_TEST}
)
set(UNIT_TEST ReadValidTagHeader)
add_custom_command(
     TARGET ${UNIT_TEST}
     COMMENT "Run tests"
     POST_BUILD 
     COMMAND ${UNIT_TEST}
)

# Edit and Continue in VS
if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "19.15")
  target_compile_options(tddfile PUBLIC "/ZI")
  target_link_options(tddfile PUBLIC "/INCREMENTAL")
endif()

if(MSVC)
  target_compile_options(tddfile PRIVATE /W4)
else()
  target_compile_options(tddfile PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()




