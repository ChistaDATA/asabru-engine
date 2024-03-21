cmake_minimum_required(VERSION 3.15)
project(AsabruEngine)
# Enables generation of compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set DEBUG preprocessor flag
add_compile_definitions(DEBUG)

# Add the subdirectories for source code and tests
add_subdirectory(src)
add_subdirectory(tests)


