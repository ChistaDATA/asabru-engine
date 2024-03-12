cmake_minimum_required(VERSION 3.15)

if (POLICY CMP0079)
    cmake_policy(SET CMP0079 NEW)
endif ()

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

MACRO(HEADER_DIRECTORIES search_path return_list)
    FILE(GLOB_RECURSE new_list ${search_path})
    SET(dir_list "")
    FOREACH (file_path ${new_list})
        GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
        SET(dir_list ${dir_list} ${dir_path})
    ENDFOREACH ()
    LIST(REMOVE_DUPLICATES dir_list)
    SET(${return_list} ${dir_list})
ENDMACRO()

HEADER_DIRECTORIES("../../asabru-commons/src/*.h" header_dir_list)
include_directories(${header_dir_list})
HEADER_DIRECTORIES("../src/**/*.h" header_dir_list)
include_directories(${header_dir_list})

include_directories(../include)
# Add your test files
set(TEST_SOURCES
        tests.h
        test_basic.h
        TestConnection.h
        tests.cpp
        main.cpp
)

# Create an executable for the tests
add_executable(tests ${TEST_SOURCES})

## Link the test executable with the library and GTest
target_link_libraries(tests asabru_engine)

target_compile_definitions(tests PRIVATE UNITTEST=True)

# Enable testing
enable_testing()

# Add the test
add_test(NAME unit_tests COMMAND tests)
