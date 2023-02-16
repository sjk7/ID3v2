cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)


project(ID3Tags LANGUAGES CXX)


# 'Just My Code' in VS
if( CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "19.15" )
    add_compile_options( /JMC)
endif()

enable_testing()
add_executable(ID3Tags src/main.cpp include/myUtils.hpp)

# Edit and Continue in VS
if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "19.15")
  target_compile_options(ID3Tags PUBLIC "/ZI")
  target_link_options(ID3Tags PUBLIC "/INCREMENTAL")
endif()


install(TARGETS ID3Tags
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

add_executable (tddfile Tests/tddfile.cpp)
add_executable(tddFileReader Tests/tddFileReader.cpp)

add_test(NAME TestFile COMMAND tddfile)
add_test(NAME FileReader COMMAND tddFileReader)

set(UNIT_TEST tddfile)
add_custom_command(
     TARGET ${UNIT_TEST}
     COMMENT "Run tests"
     POST_BUILD 
     COMMAND ${UNIT_TEST}
)

set(UNIT_TEST2 tddFileReader)
add_custom_command(
     TARGET ${UNIT_TEST2}
     COMMENT "Run tests"
     POST_BUILD 
     COMMAND ${UNIT_TEST2}
)

# Edit and Continue in VS
if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "19.15")
  target_compile_options(tddfile PUBLIC "/ZI")
  target_link_options(tddfile PUBLIC "/INCREMENTAL")
endif()

if(MSVC)
  target_compile_options(tddfile PRIVATE /W4)
else()
  target_compile_options(tddfile PRIVATE -Wall -Wextra -Wpedantic)
endif()




