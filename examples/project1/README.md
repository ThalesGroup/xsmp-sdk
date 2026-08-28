# xsmp_example_project1


This is a sample project that demonstrate the use of the xsmp-sdk framework

## System Requirements

- Linux or MacOS
- A compiler with support for C++ 17 (at least GCC 7 or Clang 5)
- CMake 3.14 or newer
- Ninja or Make
- Python 3.7 or newer (for unit-tests)

## How to Build

```bash
cmake -B ./build -DCMAKE_BUILD_TYPE=Release
cmake --build ./build --config Release
```

## How to Test

The Python tests under `python/` are discovered by CTest:

```bash
ctest --test-dir ./build
```

