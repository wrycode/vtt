cmake_minimum_required(VERSION 3.5)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(vtt)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Must run 'just google_compile' first 
link_directories(${CMAKE_CURRENT_BINARY_DIR}/google-install/lib)
set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_BINARY_DIR}/google-install/lib/cmake)
find_package(google_cloud_cpp_speech REQUIRED)

# Code testing Google's client libraries 
add_executable(stream stream.cpp)
target_link_libraries(stream google-cloud-cpp::speech)
target_include_directories(stream PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/google-install/include/)
add_dependencies(stream google-cloud-cpp::speech)

# Add executable for write.cpp
add_executable(write write.cpp)
target_compile_options(write PRIVATE -Wsign-conversion -Wall -Wextra -Wconversion)
target_include_directories(write PRIVATE ${PROJECT_SOURCE_DIR}/extern/plog/include)

# symlink of compilation database for IDEs
add_custom_command(TARGET stream POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E create_symlink
                   ${CMAKE_BINARY_DIR}/compile_commands.json
                   ${CMAKE_SOURCE_DIR}/compile_commands.json)

