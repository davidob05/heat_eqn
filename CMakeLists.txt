cmake_minimum_required(VERSION 3.16)

project(HeatEqnSolver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.4
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(Catch2)
include(CTest)


if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_library(heat_solver src/solver/heat_solver.cpp)
target_include_directories(heat_solver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(Main src/main.cpp)
add_executable(unit_tests tests/test_solver.cpp)

target_link_libraries(Main PRIVATE heat_solver)
target_link_libraries(unit_tests PRIVATE Catch2::Catch2WithMain heat_solver)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

include(Catch)
catch_discover_tests(unit_tests)

