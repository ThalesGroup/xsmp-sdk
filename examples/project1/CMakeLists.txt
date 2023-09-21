cmake_minimum_required(VERSION 3.14)

set(PARENT_PROJECT_NAME "${PROJECT_NAME}")

project(
    xsmp-examples-project1
#   VERSION 1.0.0
#   DESCRIPTION ""
#   HOMEPAGE_URL ""
    LANGUAGES CXX)

if(NOT PARENT_PROJECT_NAME)
    include(FetchContent)
    FetchContent_Declare(
        xsmp-sdk
        GIT_REPOSITORY https://github.com/ThalesGroup/xsmp-sdk.git
        GIT_TAG        main # replace with a specific tag
    )
    
    FetchContent_MakeAvailable(xsmp-sdk)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${xsmp-sdk_SOURCE_DIR}/cmake)
endif()

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS src/*.cpp src-gen/*.cpp)

add_library(xsmp_example_project1 SHARED ${SRC})
target_include_directories(xsmp_example_project1 PUBLIC src src-gen)
target_link_libraries(xsmp_example_project1 PUBLIC Xsmp::Cdk)


# --------------------------------------------------------------------

include(CTest)

# Check for CMake version >= 3.20 to execute Python tests
if(NOT "${CMAKE_VERSION}" VERSION_LESS "3.20")
    include(Pytest)
    pytest_discover_tests(xsmp_example_project1Test)
else()
     message(WARNING "Warning: Python tests will not be executed because CMake version is less than 3.20.")
endif()