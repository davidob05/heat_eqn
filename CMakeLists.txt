cmake_minimum_required(VERSION 3.16)

project(HeatEqnSolver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
add_compile_options(/W4)
else()
add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_executable(Main src/main.cpp)

add_executable(Test tests/main.cpp)

#add_library(Solver src/solver/main.cpp)


