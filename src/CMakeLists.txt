cmake_minimum_required(VERSION 3.15)
project(asabru_engine)
# Enables generation of compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(../include)
include_directories(../../libuv/include)
include(CMakePrintHelpers)
# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add compiler warnings
if (CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(-Wall)
endif ()

# Set DEBUG preprocessor flag
add_compile_definitions(DEBUG)

# Find OpenSSL library
find_package(OpenSSL REQUIRED)

# Find SqlLite3
find_package (SQLite3 REQUIRED)
include_directories(${SQLite3_INCLUDE_DIRS})

# Collect all source files
file(GLOB_RECURSE SOURCE_FILES "./**/*.cpp")

# Add build directory
set(BUILD_DIRECTORY "build")

# Set library directory
set(LIB_DIRECTORY "lib")

MACRO(HEADER_DIRECTORIES search_path return_list)
    FILE(GLOB_RECURSE new_list ${search_path})
    SET(dir_list "")
    FOREACH(file_path ${new_list})
        GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
        SET(dir_list ${dir_list} ${dir_path})
    ENDFOREACH()
    LIST(REMOVE_DUPLICATES dir_list)
    SET(${return_list} ${dir_list})
ENDMACRO()

HEADER_DIRECTORIES("../../asabru-commons/src/*.h" header_dir_list)
include_directories(${header_dir_list})
include_directories(include)

# Create a library target
add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

# Set output directory for libraries
set_target_properties(${PROJECT_NAME}
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${LIB_DIRECTORY}
)

target_link_libraries(
        ${PROJECT_NAME}
        PUBLIC OpenSSL::SSL
        PUBLIC ${SQLite3_LIBRARIES}
        ${CMAKE_CURRENT_SOURCE_DIR}/../../libuv/build/libuv.a
        ${CMAKE_CURRENT_SOURCE_DIR}/../../asabru-commons/build/lib/libasabru_commons.a
)
