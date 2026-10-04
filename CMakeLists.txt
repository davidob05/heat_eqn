cmake_minimum_required(VERSION 3.25)

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

add_library(heat_solver src/core/grid.cpp src/core/initialise_rod.cpp src/verifier/verifier.cpp)
target_include_directories(heat_solver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(Main src/main.cpp)
add_executable(unit_tests tests/test_grid.cpp tests/test_initialise_rod.cpp tests/test_verifier.cpp)

foreach(target IN ITEMS heat_solver Main unit_tests)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 $<$<CONFIG:Debug>:/WX>)
    else()
        target_compile_options(${target} PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion
            $<$<CONFIG:Debug>:-Werror>
            $<$<CONFIG:Debug>:-fsanitize=address>
            $<$<CONFIG:Debug>:-fno-omit-frame-pointer>
        )
    endif()
endforeach()

foreach(target IN ITEMS Main unit_tests)
    if(NOT MSVC)
        target_link_options(${target} PRIVATE $<$<CONFIG:Debug>:-fsanitize=address>)
    endif()
endforeach()

target_link_libraries(Main PRIVATE heat_solver)
target_link_libraries(unit_tests PRIVATE Catch2::Catch2WithMain heat_solver)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

include(Catch)
catch_discover_tests(unit_tests)

