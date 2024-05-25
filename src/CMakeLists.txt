cmake_minimum_required(VERSION 3.15)
project(asabru_engine)
# Enables generation of compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

# add dependencies
include(../cmake/CPM.cmake)

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


include_directories(../include)

# Create a library target
add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME}
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

# Set output directory for libraries
set_target_properties(${PROJECT_NAME}
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${LIB_DIRECTORY}
)

if (ASABRU_COMMONS_BUILD STREQUAL "GIT_TAG")
    CPMAddPackage(
            NAME asabru_commons
            GITHUB_REPOSITORY ChistaDATA/asabru-commons
            GIT_TAG ${ASABRU_COMMONS_GIT_TAG}
    )
elseif (ASABRU_COMMONS_BUILD STREQUAL "LOCAL_DIR")
    # For development purpose
    CPMAddPackage(
            NAME asabru_commons
            SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../asabru-commons/src
    )
else()
    CPMAddPackage(
            NAME asabru_commons
            GITHUB_REPOSITORY ChistaDATA/asabru-commons
            VERSION 0.0.3
    )
endif()


CPMAddPackage(
        NAME libuv
        GITHUB_REPOSITORY libuv/libuv
        VERSION 1.48.0
)

target_link_libraries(
        ${PROJECT_NAME}
        PUBLIC OpenSSL::SSL
        PUBLIC ${SQLite3_LIBRARIES}
        PUBLIC uv_a
        PUBLIC asabru_commons
)

target_include_directories(${PROJECT_NAME} PUBLIC ${libuv_SOURCE_DIR}/include)
