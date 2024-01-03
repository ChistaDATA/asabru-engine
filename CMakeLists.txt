cmake_minimum_required(VERSION 3.15)
project(AsabruEngine)

# Set DEBUG preprocessor flag
add_compile_definitions(DEBUG)

# Add the subdirectories for source code and tests
add_subdirectory(src)
add_subdirectory(tests)


