cmake_minimum_required(VERSION 3.3.0)
project(steering VERSION 0.1.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SOURCE
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/Game.cpp
)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/external
)

add_executable(${PROJECT_NAME} ${SOURCE})

find_package(SDL2)
find_package(SDL2_image REQUIRED)

find_package(glm REQUIRED)

target_link_libraries(${PROJECT_NAME}
    SDL2::SDL2
    SDL2_image::SDL2_image
)

include_directories(${PROJECT_NAME}
    ${GLM_INCLUDE_DIRS}
)

# Unit Test
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.13.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP true
)

if (WIN32)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()
FetchContent_MakeAvailable(googletest)

enable_testing()

set(TEST test_steering)
add_executable(${TEST} ${CMAKE_SOURCE_DIR}/test/TestTransformation.cpp)

target_link_libraries(${TEST} gtest_main)
include_directories(${TEST} ${CMAKE_SOURCE_DIR}/src)

add_test(NAME ${TEST} COMMAND ${TEST})
