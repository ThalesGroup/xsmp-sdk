
cmake_minimum_required(VERSION 3.20)

if(NOT "${CMAKE_VERSION}" VERSION_LESS "3.27")
  cmake_policy(SET CMP0144 NEW)
endif()

find_package(Python COMPONENTS Interpreter Development)

function(pytest_discover_tests NAME)
    cmake_parse_arguments(
        PARSE_ARGV 1 "" ""
        "WORKING_DIRECTORY;TRIM_FROM_NAME;BUNDLE_TESTS"
        "LIBRARY_PATH_PREPEND;PYTHON_PATH_PREPEND;ENVIRONMENT"
    )

    # Set library path depending on the platform.
    if (CMAKE_SYSTEM_NAME STREQUAL Windows)
        set(LIB_ENV_PATH PATH)
        set(_env_sep "\\\\\;")
    elseif(CMAKE_SYSTEM_NAME STREQUAL Darwin)
        set(LIB_ENV_PATH DYLD_LIBRARY_PATH)
        set(_env_sep ":")
    else()
        set(LIB_ENV_PATH LD_LIBRARY_PATH)
        set(_env_sep ":")
    endif()

    # Convert all paths into cmake paths.
    cmake_path(CONVERT "$ENV{${LIB_ENV_PATH}}" TO_CMAKE_PATH_LIST libpath)
    cmake_path(CONVERT "$ENV{PYTHONPATH}" TO_CMAKE_PATH_LIST pythonpath)

    # Prepend input path to environment variables
    if (_LIBRARY_PATH_PREPEND)
        list(REVERSE _LIBRARY_PATH_PREPEND)
        foreach (_path ${_LIBRARY_PATH_PREPEND})
            set(libpath "${_path}" "${libpath}")
        endforeach()
    endif()

    get_property(GENERATOR_IS_MULTI_CONFIG GLOBAL
        PROPERTY GENERATOR_IS_MULTI_CONFIG
    )

    if(GENERATOR_IS_MULTI_CONFIG)
      set(libpath "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/$<CONFIG>" "${libpath}")
      set(pythonpath "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/$<CONFIG>" "${xsmp-sdk_SOURCE_DIR}/python" "${pythonpath}")
    else()
      set(libpath "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}" "${libpath}")
      set(pythonpath "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}" "${xsmp-sdk_SOURCE_DIR}/python" "${pythonpath}")
    endif()

    if (_PYTHON_PATH_PREPEND)
        list(REVERSE _PYTHON_PATH_PREPEND)
        foreach (_path ${_PYTHON_PATH_PREPEND})
            set(pythonpath "${_path}" "${pythonpath}")
        endforeach()
    endif()

    # Convert list into string.
    list(JOIN libpath "${_env_sep}" libpath)
    list(JOIN pythonpath "${_env_sep}" pythonpath)

    if (NOT _WORKING_DIRECTORY)
        set(_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/python)
    endif()
    if (NOT _TRIM_FROM_NAME)
        set(_TRIM_FROM_NAME "^test_")
    endif()
    # Override option by environment variable if available.
    if (DEFINED ENV{BUNDLE_PYTHON_TESTS})
        set(_BUNDLE_TESTS $ENV{BUNDLE_PYTHON_TESTS})
    endif()


    set(ctest_file_base "${CMAKE_CURRENT_BINARY_DIR}/${NAME}")
    set(ctest_include_file "${ctest_file_base}_include.cmake")
  

    if(GENERATOR_IS_MULTI_CONFIG)
      set(ctest_tests_file "${ctest_file_base}_tests-$<CONFIG>.cmake")
    else()
      set(ctest_tests_file "${ctest_file_base}_tests.cmake")
    endif()
    string(CONCAT ctest_include_content

      "include(\"${_PYTEST_DISCOVER_TESTS_SCRIPT}\")"                       "\n"
      "pytest_discover_tests_impl("                                         "\n"
      "    PYTHON_EXECUTABLE"        " [==[" "${Python_EXECUTABLE}"  "]==]" "\n"
      "    TEST_GROUP_NAME"          " [==[" "${NAME}"               "]==]" "\n"
      "    BUNDLE_TESTS"             " [==[" "${_BUNDLE_TESTS}"      "]==]" "\n"
      "    LIB_ENV_PATH"             " [==[" "${LIB_ENV_PATH}"       "]==]" "\n"
      "    LIBRARY_PATH"             " [==[" "${libpath}"            "]==]" "\n"
      "    PYTHON_PATH"              " [==[" "${pythonpath}"         "]==]" "\n"
      "    TRIM_FROM_NAME"           " [==[" "${_TRIM_FROM_NAME}"    "]==]" "\n"
      "    WORKING_DIRECTORY"        " [==[" "${_WORKING_DIRECTORY}" "]==]" "\n"
      "    ENVIRONMENT"              " [==[" "${_ENVIRONMENT}"       "]==]" "\n"
      "    PROJECT_SOURCE_DIR"       " [==[" "${PROJECT_SOURCE_DIR}" "]==]" "\n"
      "    CTEST_FILE"               " [==[" "${ctest_tests_file}"    "]==]" "\n"
      ")"                                                                   "\n"
      "include(\"${ctest_tests_file}\")"                                     "\n"

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

