cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)
project(ID3Tags LANGUAGES CXX)
include(CTest) 



# warning levels, JustMyCode, Edit and Continue:
include(${CMAKE_CURRENT_SOURCE_DIR}/config/CMakeLists.txt)


enable_testing()
add_executable(ID3HeaderApp "Tests/tddTagReader.cpp" include/myUtils.hpp include/ID3v2.hpp include/FileDataReader.hpp include/IDataReader.hpp)

#install(TARGETS ID3HeaderApp
#    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

add_executable (tddfile Tests/tddfile.cpp include/myUtils.hpp)

add_executable(tddFileReader Tests/tddFileReader.cpp include/FileDataReader.hpp
    include/IDataReader.hpp)

add_executable(BadID3HeaderTest Tests/tddID3Header.cpp include/myUtils.hpp include/ID3v2.hpp
include/FileDataReader.hpp include/IDataReader.hpp)

add_executable(ReadValidTagHeader Tests/tddReadValidTagHeader.cpp include/ID3v2.hpp)
add_executable(TestID3Skipper Tests/tddID3Skipper.cpp)
add_executable(TagReader Tests/tddTagReader.cpp)

add_test(NAME TestFile COMMAND tddfile)
add_test(NAME FileReader COMMAND tddFileReader)
add_test(NAME TestBadHeader COMMAND BadID3HeaderTest)
add_test(NAME ReadValidHeaderTag COMMAND ReadValidTagHeader)
add_test(NAME ID3Skipper COMMAND TestID3Skipper)
add_test(NAME TagReadAll COMMAND TagReader)

list(APPEND CMAKE_CTEST_ARGUMENTS "--output-on-failure")


add_custom_command(
        TARGET ID3HeaderApp PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/TestFileSamples/BustedButReadableTag.mp3
                ${CMAKE_CURRENT_BINARY_DIR}/BustedButReadableTag.mp3)

                add_custom_command(
        TARGET ID3HeaderApp PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/TestFileSamples/sample.mp3
                ${CMAKE_CURRENT_BINARY_DIR}/sample.mp3)

                add_custom_command(
        TARGET ID3HeaderApp PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/TestFileSamples/sampleSAVEDINDPS.mp3
                ${CMAKE_CURRENT_BINARY_DIR}/sampleSAVEDINDPS.mp3)

                                add_custom_command(
        TARGET ID3HeaderApp PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/TestFileSamples/testfile.bin
                ${CMAKE_CURRENT_BINARY_DIR}/testfile.bin)


               







