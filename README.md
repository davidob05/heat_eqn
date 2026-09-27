# heat_eqn
A program designed to approximate the solutions to the heat equation


# Instructions

# Requirements
 - C++17 compiler
 - CMake 3.25 or newer 
 - First configure downloads Catch2 so internet is needed

## Build debug
```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
./build/debug/Main
```

## Build release
```bash
cmake --preset release
cmake --build --preset release
ctest --preset release
./build/release/Main
```

## Debug vs Release
Debug is for development, release is for real, optimised runs.

\n