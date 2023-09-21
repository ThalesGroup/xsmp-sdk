
cmake_minimum_required(VERSION 3.14)

function(pytest_discover_tests_impl)

    cmake_parse_arguments(
        ""
        ""
        "PYTHON_EXECUTABLE;TEST_GROUP_NAME;BUNDLE_TESTS;LIB_ENV_PATH;LIBRARY_PATH;PYTHON_PATH;TRIM_FROM_NAME;WORKING_DIRECTORY;ENVIRONMENT;PROJECT_SOURCE_DIR;CTEST_FILE"
        ""
        ${ARGN}
    )

    set(_content "")
    

    # Convert input environment variable into list.
    string(REPLACE "\\\;" ";" _LIBRARY_PATH "${_LIBRARY_PATH}")
    string(REPLACE "\\\;" ";" _PYTHON_PATH "${_PYTHON_PATH}")

    if (_BUNDLE_TESTS)
        string(APPEND _content
            "add_test(\"${_TEST_GROUP_NAME}\" ${_PYTHON_EXECUTABLE} -m pytest \"${_WORKING_DIRECTORY}\"\)\n"
            "set_tests_properties(\"${_TEST_GROUP_NAME}\" PROPERTIES ENVIRONMENT \"${_LIB_ENV_PATH}=${_LIBRARY_PATH}\")\n"
            "set_tests_properties(\"${_TEST_GROUP_NAME}\" APPEND PROPERTIES ENVIRONMENT \"PYTHONPATH=${_PYTHON_PATH}\")\n"
        )

        foreach(env ${_ENVIRONMENT})
            string(APPEND _content
                "set_tests_properties(\"${_TEST_GROUP_NAME}\" APPEND PROPERTIES ENVIRONMENT ${env})\n"
            )
        endforeach()

    else()
        # save paths
        set(old_lib_env_path "ENV{${_LIB_ENV_PATH}}")
        set(old_python_path "ENV{${PYTHONPATH}}")
        # collect tests
        set(ENV{${_LIB_ENV_PATH}} "${_LIBRARY_PATH}")
        set(ENV{PYTHONPATH} "${_PYTHON_PATH}")
        execute_process(
            COMMAND ${_PYTHON_EXECUTABLE} -m pytest --collect-only -q --rootdir=${_WORKING_DIRECTORY} .
            OUTPUT_VARIABLE _output_list
            ERROR_VARIABLE _output_list
            OUTPUT_STRIP_TRAILING_WHITESPACE
            WORKING_DIRECTORY ${_WORKING_DIRECTORY}
        )
        #restore paths
        set(ENV{${_LIB_ENV_PATH}} "${old_lib_env_path}")
        set(ENV{PYTHONPATH} "${old_python_path}")

        # Parse output
        string(REPLACE [[;]] [[\;]] _output_list "${_output_list}")
        string(REPLACE "\n" ";" _output_list "${_output_list}")

        set(test_pattern "([^:]+)(::([^:]+))?::([^:]+)")

        foreach (test_case ${_output_list})
            string(REGEX MATCHALL ${test_pattern} _test_case "${test_case}")

            # Ignore lines not identified as a test.
            if (NOT _test_case)
                continue()
            endif()

            set(_class ${CMAKE_MATCH_3})
            set(_func ${CMAKE_MATCH_4})

            if (_class)
                set(test_name "${_class}.${_func}")
            else()
                set(test_name "${_func}")
            endif()

            if (_TRIM_FROM_NAME)
                string(REGEX REPLACE
                        "${_TRIM_FROM_NAME}" "" test_name "${test_name}")
            endif()

            set(test_name "${_TEST_GROUP_NAME}.${test_name}")
            set(test_case "${_WORKING_DIRECTORY}/${test_case}")

            string(APPEND _content
                "add_test(\"${test_name}\" ${_PYTHON_EXECUTABLE} -m pytest \"${test_case}\")\n"
                "set_tests_properties(\"${test_name}\" PROPERTIES ENVIRONMENT \"${_LIB_ENV_PATH}=${_LIBRARY_PATH}\")\n"
                "set_tests_properties(\"${test_name}\" APPEND PROPERTIES ENVIRONMENT \"PYTHONPATH=${_PYTHON_PATH}\")\n"
            )

            foreach(env ${_ENVIRONMENT})
                string(APPEND _content
                    "set_tests_properties(\"${test_name}\" APPEND PROPERTIES ENVIRONMENT ${env})\n"
                )
            endforeach()

        endforeach()

        if(NOT _content)
            string(APPEND _content
                "add_test(\"${_TEST_GROUP_NAME}\" ${_PYTHON_EXECUTABLE} -m pytest \"${_WORKING_DIRECTORY}\"\)\n"
                "set_tests_properties(\"${_TEST_GROUP_NAME}\" PROPERTIES ENVIRONMENT \"${_LIB_ENV_PATH}=${_LIBRARY_PATH}\")\n"
                "set_tests_properties(\"${_TEST_GROUP_NAME}\" APPEND PROPERTIES ENVIRONMENT \"PYTHONPATH=${_PYTHON_PATH}\")\n"
            )
    
            foreach(env ${_ENVIRONMENT})
                string(APPEND _content
                    "set_tests_properties(\"${_TEST_GROUP_NAME}\" APPEND PROPERTIES ENVIRONMENT ${env})\n"
                )
            endforeach()
        endif()
    endif()

    file(WRITE ${_CTEST_FILE} ${_content})
endfunction()

if(CMAKE_SCRIPT_MODE_FILE)

    pytest_discover_tests_impl(
        PYTHON_EXECUTABLE ${PYTHON_EXECUTABLE}
        TEST_GROUP_NAME ${TEST_GROUP_NAME}
        BUNDLE_TESTS ${BUNDLE_TESTS}
        LIB_ENV_PATH ${LIB_ENV_PATH}
        LIBRARY_PATH ${LIBRARY_PATH}
        PYTHON_PATH ${PYTHON_PATH}
        TRIM_FROM_NAME ${TRIM_FROM_NAME}
        WORKING_DIRECTORY ${WORKING_DIRECTORY}
        ENVIRONMENT ${ENVIRONMENT}
        PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR}
        CTEST_FILE ${CTEST_FILE}
    )
endif(CMAKE_SCRIPT_MODE_FILE)
