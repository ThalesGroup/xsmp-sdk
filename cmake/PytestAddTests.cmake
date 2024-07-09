
cmake_minimum_required(VERSION 3.14)

function(pytest_discover_tests_impl)

    cmake_parse_arguments(
        ""
        ""
        "PYTHON_EXECUTABLE;TEST_PROJECT_NAME;TEST_GROUP_NAME;BUNDLE_TESTS;LIB_ENV_PATH;LIBRARY_PATH;PYTHON_PATH;WORKING_DIRECTORY;ENVIRONMENT;CTEST_FILE"
        ""
        ${ARGN}
    )

    set(_content "")
    
    if(CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
        string(REPLACE "][" [[\\;]] _LIBRARY_PATH "${_LIBRARY_PATH}")
        string(REPLACE "][" [[\\;]] _PYTHON_PATH "${_PYTHON_PATH}")
    else()
        string(REPLACE "][" [[:]] _LIBRARY_PATH "${_LIBRARY_PATH}")
        string(REPLACE "][" [[:]] _PYTHON_PATH "${_PYTHON_PATH}")
    endif()
    
    # Override option by environment variable if available.
    if(DEFINED ENV{BUNDLE_PYTHON_TESTS})
        set(_BUNDLE_TESTS $ENV{BUNDLE_PYTHON_TESTS})
    endif()

    if(_BUNDLE_TESTS)
        if(NOT _TEST_GROUP_NAME)
            set(_TEST_GROUP_NAME "pytest ${_WORKING_DIRECTORY}")
        endif()
        string(APPEND _content
            "add_test(\"${_TEST_GROUP_NAME}\" ${_PYTHON_EXECUTABLE} -m pytest)\n"
            "set_tests_properties(\"${_TEST_GROUP_NAME}\" PROPERTIES WORKING_DIRECTORY [==[${_WORKING_DIRECTORY}]==])\n"
            "set_tests_properties(\"${_TEST_GROUP_NAME}\" PROPERTIES ENVIRONMENT [==[${_LIB_ENV_PATH}=${_LIBRARY_PATH}]==])\n"
            "set_tests_properties(\"${_TEST_GROUP_NAME}\" PROPERTIES ENVIRONMENT [==[PYTHONPATH=${_PYTHON_PATH}]==])\n"
        )

        foreach(env ${_ENVIRONMENT})
            string(APPEND _content
                "set_tests_properties(\"${_TEST_GROUP_NAME}\" APPEND PROPERTIES ENVIRONMENT [==[${env}]==])\n"
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
            COMMAND ${_PYTHON_EXECUTABLE} -m pytest --collect-only -q
            OUTPUT_VARIABLE _output
            ERROR_VARIABLE _output
            OUTPUT_STRIP_TRAILING_WHITESPACE
            WORKING_DIRECTORY ${_WORKING_DIRECTORY}
        )
        #restore paths
        set(ENV{${_LIB_ENV_PATH}} "${old_lib_env_path}")
        set(ENV{PYTHONPATH} "${old_python_path}")

        # Parse output
        string(REPLACE [[;]] [[\;]] _output "${_output}")
        string(REPLACE "\n" ";" _output "${_output}")

        set(test_pattern "([^:]+)(::([^:]+))?::([^:]+)")
        set(test_error_pattern "^ERROR ([^:]+)")

        foreach(test_case ${_output})
            string(REGEX MATCHALL ${test_pattern} _test_case "${test_case}")

            # Ignore lines not identified as a test.
            if(NOT _test_case)
                string(REGEX MATCHALL ${test_error_pattern} _test_case "${test_case}")
                if(_test_case)
                    set(test_name "pytest ${_WORKING_DIRECTORY}/${CMAKE_MATCH_1}")
                    string(APPEND _content 
                        "add_test(\"${test_name}\" ${_PYTHON_EXECUTABLE} -m pytest \"${test_name}\")\n"
                        "set_tests_properties(\"${test_name}\" PROPERTIES WORKING_DIRECTORY [==[${_WORKING_DIRECTORY}]==])\n"
                        "set_tests_properties(\"${test_name}\" PROPERTIES ENVIRONMENT [==[${_LIB_ENV_PATH}=${_LIBRARY_PATH}]==])\n"
                        "set_tests_properties(\"${test_name}\" PROPERTIES ENVIRONMENT [==[PYTHONPATH=${_PYTHON_PATH}]==])\n"
                    )
                    foreach(env ${_ENVIRONMENT})
                        string(APPEND _content
                            "set_tests_properties(\"${test_name}\" APPEND PROPERTIES ENVIRONMENT [==[${env}]==])\n"
                        )
                    endforeach()
                endif(_test_case)
                continue()
            endif()

            set(_class ${CMAKE_MATCH_3})
            set(_func ${CMAKE_MATCH_4})


            string(REGEX REPLACE "^test_?" "" _func "${_func}")

            if(_class)
                string(REGEX REPLACE "^Test|Test$" "" _class "${_class}")
                set(test_name "${_class}.${_func}")
            else()
                set(test_name "${_func}")
            endif()


            if(_TEST_GROUP_NAME)
                set(test_name "${_TEST_GROUP_NAME}:${test_name}")
            endif()
            set(test_case "${_WORKING_DIRECTORY}/${test_case}")

            string(APPEND _content
                "add_test(\"${test_name}\" ${_PYTHON_EXECUTABLE} -m pytest \"${test_case}\")\n"
                "set_tests_properties(\"${test_name}\" PROPERTIES WORKING_DIRECTORY [==[${_WORKING_DIRECTORY}]==])\n"
                "set_tests_properties(\"${test_name}\" PROPERTIES ENVIRONMENT [==[${_LIB_ENV_PATH}=${_LIBRARY_PATH}]==])\n"
                "set_tests_properties(\"${test_name}\" PROPERTIES ENVIRONMENT [==[PYTHONPATH=${_PYTHON_PATH}]==])\n"
            )

            foreach(env ${_ENVIRONMENT})
                string(APPEND _content
                    "set_tests_properties(\"${test_name}\" APPEND PROPERTIES ENVIRONMENT [==[${env}]==])\n"
                )
            endforeach()

        endforeach()

        if(NOT _content)
            string(APPEND _content
                "add_test(\"${_WORKING_DIRECTORY}\" ${_PYTHON_EXECUTABLE} -m pytest)\n"
                "set_tests_properties(\"${_WORKING_DIRECTORY}\" PROPERTIES WORKING_DIRECTORY [==[${_WORKING_DIRECTORY}]==])\n"
                "set_tests_properties(\"${_WORKING_DIRECTORY}\" PROPERTIES ENVIRONMENT [==[${_LIB_ENV_PATH}=${_LIBRARY_PATH}]==])\n"
                "set_tests_properties(\"${_WORKING_DIRECTORY}\" PROPERTIES ENVIRONMENT [==[PYTHONPATH=${_PYTHON_PATH}]==])\n"
            )
    
            foreach(env ${_ENVIRONMENT})
                string(APPEND _content
                    "set_tests_properties(\"${_WORKING_DIRECTORY}\" APPEND PROPERTIES ENVIRONMENT [==[${env}]==])\n"
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
        WORKING_DIRECTORY ${WORKING_DIRECTORY}
        ENVIRONMENT ${ENVIRONMENT}
        CTEST_FILE ${CTEST_FILE}
    )
endif(CMAKE_SCRIPT_MODE_FILE)
