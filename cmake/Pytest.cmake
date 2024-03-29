cmake_minimum_required(VERSION 3.14)
 
if(NOT "${CMAKE_VERSION}" VERSION_LESS "3.27")
  cmake_policy(SET CMP0144 NEW)
endif()

find_package(Python COMPONENTS Interpreter Development)

function(pytest_discover_tests)
    cmake_parse_arguments(
        PARSE_ARGV 0 "" ""
        "NAME;WORKING_DIRECTORY;BUNDLE_TESTS"
        "LIBRARY_PATH_PREPEND;PYTHON_PATH_PREPEND;ENVIRONMENT"
    )

    # Set library/python path depending on the platform.
    if (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
        set(LIB_ENV_PATH PATH)
        string(REPLACE "\\" "/" libpath "$ENV{${LIB_ENV_PATH}}")
        string(REPLACE "\\" "/" pythonpath "$ENV{PYTHONPATH}")
    else()
        if(CMAKE_HOST_SYSTEM_NAME STREQUAL Darwin)
            set(LIB_ENV_PATH DYLD_LIBRARY_PATH)
        else()
            set(LIB_ENV_PATH LD_LIBRARY_PATH)
        endif()
        string(REPLACE ":" ";" libpath "$ENV{${LIB_ENV_PATH}}")
        string(REPLACE ":" ";" pythonpath "$ENV{PYTHONPATH}")
    endif()


    # Prepend input path to environment variables
    if (_LIBRARY_PATH_PREPEND)
        list(REVERSE _LIBRARY_PATH_PREPEND)
        foreach (_path ${_LIBRARY_PATH_PREPEND})
            list(INSERT libpath 0 "${_path}")
        endforeach()
    endif()

    get_property(GENERATOR_IS_MULTI_CONFIG GLOBAL
        PROPERTY GENERATOR_IS_MULTI_CONFIG
    )
    
    # update PATH and PYTHONPATH in case the user use a custom CMAKE_INSTALL_PREFIX
    if(NOT CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        list(INSERT libpath 0 "${CMAKE_INSTALL_PREFIX}/lib")
        if (NOT CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
            list(INSERT pythonpath 0 "${CMAKE_INSTALL_PREFIX}${Python_SITELIB}")
        endif()
    endif()
    
    #add xsmp-sdk python folder to python path
    list(INSERT pythonpath 0 "${xsmp-sdk_SOURCE_DIR}/python")
    
    if(GENERATOR_IS_MULTI_CONFIG)
        list(INSERT libpath 0 "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/$<CONFIG>")
        list(INSERT pythonpath 0 "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/$<CONFIG>")
    else()
        list(INSERT libpath 0 "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
        list(INSERT pythonpath 0 "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
    endif()

    if (_PYTHON_PATH_PREPEND)
        list(REVERSE _PYTHON_PATH_PREPEND)
        foreach (_path ${_PYTHON_PATH_PREPEND})
            list(INSERT pythonpath 0 "${_path}")
        endforeach()
    endif()

    # Convert list into string.
    list(JOIN libpath "][" libpath)
    list(JOIN pythonpath "][" pythonpath)

    if (NOT _WORKING_DIRECTORY)
        set(_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/python)
    endif()

    # Override option by environment variable if available.
    if (DEFINED ENV{BUNDLE_PYTHON_TESTS})
        set(_BUNDLE_TESTS $ENV{BUNDLE_PYTHON_TESTS})
    endif()

    if (NOT _NAME)
        set(_NAME "py")
    endif()
    
    set(ctest_file_base "${CMAKE_CURRENT_BINARY_DIR}/${_NAME}")
    set(ctest_include_file "${ctest_file_base}_include.cmake")
  

    if(GENERATOR_IS_MULTI_CONFIG)
      set(ctest_tests_file "${ctest_file_base}_tests-$<CONFIG>.cmake")
    else()
      set(ctest_tests_file "${ctest_file_base}_tests.cmake")
    endif()
    string(CONCAT ctest_include_content

      "include(\"${_PYTEST_DISCOVER_TESTS_SCRIPT}\")"                    "\n"
      "pytest_discover_tests_impl("                                      "\n"
      "    PYTHON_EXECUTABLE"     " [==[" "${Python_EXECUTABLE}"  "]==]" "\n"
      "    TEST_GROUP_NAME"       " [==[" "${_NAME}"               "]==]" "\n"
      "    BUNDLE_TESTS"          " [==[" "${_BUNDLE_TESTS}"      "]==]" "\n"
      "    LIB_ENV_PATH"          " [==[" "${LIB_ENV_PATH}"       "]==]" "\n"
      "    LIBRARY_PATH"          " [==[" "${libpath}"            "]==]" "\n"
      "    PYTHON_PATH"           " [==[" "${pythonpath}"         "]==]" "\n"
      "    WORKING_DIRECTORY"     " [==[" "${_WORKING_DIRECTORY}" "]==]" "\n"
      "    ENVIRONMENT"           " [==[" "${_ENVIRONMENT}"       "]==]" "\n"
      "    CTEST_FILE"            " [==[" "${ctest_tests_file}"    "]==]" "\n"
      ")"                                                                "\n"
      "include(\"${ctest_tests_file}\")"                                  "\n"

    )

    if(GENERATOR_IS_MULTI_CONFIG)
      foreach(_config ${CMAKE_CONFIGURATION_TYPES})
        file(GENERATE OUTPUT "${ctest_file_base}_include-${_config}.cmake" CONTENT "${ctest_include_content}" CONDITION $<CONFIG:${_config}>)
      endforeach()
      file(WRITE "${ctest_include_file}" "include(\"${ctest_file_base}_include-\${CTEST_CONFIGURATION_TYPE}.cmake\")")
    else()
      file(GENERATE OUTPUT "${ctest_file_base}_include.cmake" CONTENT "${ctest_include_content}")
      file(WRITE "${ctest_include_file}" "include(\"${ctest_file_base}_include.cmake\")")
    endif()
    # Add discovered tests to directory TEST_INCLUDE_FILES
    set_property(DIRECTORY
        APPEND PROPERTY TEST_INCLUDE_FILES "${ctest_include_file}")

endfunction()

set(_PYTEST_DISCOVER_TESTS_SCRIPT
  ${CMAKE_CURRENT_LIST_DIR}/PytestAddTests.cmake
)

    
