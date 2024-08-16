cmake_minimum_required(VERSION 3.5)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
project(vtt)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

link_directories(${CMAKE_CURRENT_BINARY_DIR}/google-install/lib)
set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_BINARY_DIR}/google-install/lib/cmake)

find_package(google_cloud_cpp_speech REQUIRED)

find_package(PkgConfig)
find_package(PkgConfig REQUIRED)

pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
if (GSTREAMER_FOUND)
    message(STATUS "GStreamer found: INCLUDE ${GSTREAMER_INCLUDE_DIRS}, LIBS ${GSTREAMER_LIBRARIES}")
else()
    message(FATAL_ERROR "GStreamer not found")
endif()

include_directories(${GSTREAMER_INCLUDE_DIRS})
link_directories(${GSTREAMER_LIBRARY_DIRS})

# vtt.cpp entrypoint, currently testing various components
find_package(ICU COMPONENTS io i18n uc REQUIRED)
add_executable(vtt vtt.cpp src/UnicodeString.cpp src/TranscriptionSegment.cpp src/Keyboard.cpp)
target_include_directories(vtt PUBLIC ${ICU_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/src )
target_link_libraries(vtt PRIVATE ${ICU_LIBRARIES})

# tests
find_package(Catch2 3 REQUIRED)
add_executable(tests
  src/UnicodeString.tests.cpp src/UnicodeString.cpp
  src/TranscriptionSegment.tests.cpp src/TranscriptionSegment.cpp
  src/Keyboard.tests.cpp src/Keyboard.cpp
  src/test_common.cpp
)
target_include_directories(tests PUBLIC ${ICU_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/extern/FakeIt/single_header)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain ${ICU_LIBRARIES})

# stream.cpp, prototype to live transcribe audio from microphone
# add_executable(stream stream.cpp)
# target_link_libraries(stream PRIVATE google-cloud-cpp::speech ${GSTREAMER_LIBRARIES})
# target_include_directories(stream PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/google-install/include/ ${PORTAUDIO_INCLUDE_DIR})
# add_dependencies(stream google-cloud-cpp::speech)

# write.cpp, prototype to enter arbitrary text from a simulated keyboard
# add_executable(write write.cpp)
# target_compile_options(write PRIVATE -Wsign-conversion -Wall -Wextra -Wconversion)
# target_include_directories(write PRIVATE ${PROJECT_SOURCE_DIR}/extern/plog/include)

# symlink of compilation database for IDEs
add_custom_command(TARGET tests vtt POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E create_symlink
                   ${CMAKE_BINARY_DIR}/compile_commands.json
                   ${CMAKE_SOURCE_DIR}/compile_commands.json)

