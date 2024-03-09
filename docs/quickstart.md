# Quickstart

## How to Build


### CMake

In your project `CMakeLists.txt`, import the xsmp-sdk project:
 -  with FetchContent:
    ```cmake
    ...
    
    include(FetchContent)
    FetchContent_Declare(
      xsmp-sdk
      GIT_REPOSITORY https://github.com/ThalesGroup/xsmp-sdk.git
      GIT_TAG        main # replace main with a specific tag
    )
    
    FetchContent_MakeAvailable(xsmp-sdk)
    #use cmake modules from xsmp-sdk
    list(APPEND CMAKE_MODULE_PATH "${xsmp-sdk_SOURCE_DIR}/cmake")
    
    ...
    ```

 -  with find_package (xsmp-sdk must be installed on your OS):
    ```cmake
    ...
    
    find_package(xsmp-sdk REQUIRED)
    
    ...
    ```
Your Catalogue library need to be linked with the Xsmp::Cdk library:

```cmake
...

target_link_libraries(your_catalogue PUBLIC Xsmp::Cdk)

...
```


Python unit test can be discovered automatically:

```cmake
...

# discover python tests
include(CTest)
include(Pytest)
pytest_discover_tests()

...
```

Your final `CMakeLists.txt` should looks like this:

```cmake
cmake_minimum_required(VERSION 3.14)


project(your_project LANGUAGES CXX)

# find locally installed version of xsmp-sdk
find_package(xsmp-sdk QUIET)

# or get xsmp-sdk with FetchContent
if(NOT xsmp-sdk_FOUND ) 
    message(STATUS "xsmp-sdk is not installed, downloading it.")
    include(FetchContent)
    FetchContent_Declare(
        xsmp-sdk
        GIT_REPOSITORY https://github.com/ThalesGroup/xsmp-sdk.git
        GIT_TAG        main # replace with a specific tag
    )
        
    FetchContent_MakeAvailable(xsmp-sdk)
    #use cmake modules from xsmp-sdk
    list(APPEND CMAKE_MODULE_PATH "${xsmp-sdk_SOURCE_DIR}/cmake")
endif()

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS src/*.cpp src-gen/*.cpp)
add_library(your_catalogue SHARED ${SRC})
target_include_directories(your_catalogue PUBLIC src src-gen)
target_link_libraries(your_catalogue PUBLIC Xsmp::Cdk)

# discover python tests
include(CTest)
include(Pytest)
pytest_discover_tests()

```
