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
        GIT_TAG        v1.0.3 # replace with a specific tag
    )
    
    FetchContent_MakeAvailable(xsmp-sdk)
    list(APPEND CMAKE_MODULE_PATH "${xsmp-sdk_SOURCE_DIR}/cmake")
endif()

# add python directory to PYTHONPATH
include(PathUtils)
python_path_prepend("python")

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS src/*.cpp src-gen/*.cpp)

add_library(xsmp_example_project1 SHARED ${SRC})
target_include_directories(xsmp_example_project1 PUBLIC src src-gen)
target_link_libraries(xsmp_example_project1 PUBLIC Xsmp::Cdk)
target_compile_options(xsmp_example_project1 PRIVATE ${XSMP_COMPILE_OPTIONS})

# --------------------------------------------------------------------

include(CTest)


include(Pytest)
pytest_discover_tests()
