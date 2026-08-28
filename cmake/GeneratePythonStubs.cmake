# Copyright 2025 THALES ALENIA SPACE FRANCE. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Generate PEP 561 stubs (ecss_smp-stubs/*.pyi) for the ecss_smp extension
# module. Without them, static analysers such as Pylance/pyright - which never
# import the compiled module - cannot offer any completion on ecss_smp.
#
# xsmp_generate_python_stubs(<target> MODULE <name> OUTPUT_DIR <dir>)
function(xsmp_generate_python_stubs TARGET)
    cmake_parse_arguments(ARG "" "MODULE;OUTPUT_DIR" "" ${ARGN})

    set(_stubs_dir "${ARG_OUTPUT_DIR}/${ARG_MODULE}-stubs")
    set(_stamp "${CMAKE_CURRENT_BINARY_DIR}/${ARG_MODULE}-stubs.stamp")

    # `cmake -E rm` only exists since CMake 3.17
    if(CMAKE_VERSION VERSION_LESS 3.17)
        set(_remove_stubs_dir remove_directory "${_stubs_dir}")
    else()
        set(_remove_stubs_dir rm -rf "${_stubs_dir}")
    endif()

    # The directory of the module is prepended to PYTHONPATH rather than
    # replacing it: a wheel is built in an isolated environment whose packages,
    # pybind11-stubgen included, are reached through PYTHONPATH.
    if(WIN32)
        set(_path_separator ";")
    else()
        set(_path_separator ":")
    endif()
    set(_pythonpath "$<TARGET_FILE_DIR:${TARGET}>")
    if(NOT "$ENV{PYTHONPATH}" STREQUAL "")
        set(_pythonpath "${_pythonpath}${_path_separator}$ENV{PYTHONPATH}")
    endif()

    add_custom_command(
        OUTPUT "${_stamp}"
        COMMAND ${CMAKE_COMMAND} -E ${_remove_stubs_dir}
        COMMAND ${CMAKE_COMMAND} -E env
                "PYTHONPATH=${_pythonpath}"
                "${PYBIND11_STUBGEN_EXECUTABLE}" "${ARG_MODULE}"
                --output-dir "${ARG_OUTPUT_DIR}"
                "--root-suffix=-stubs"
                --exit-code
                # these enums are used as default argument values; pybind11
                # renders them as `<Enum.LITERAL: value>`, which is not a valid
                # python expression
                --enum-class-locations "ViewKind:${ARG_MODULE}.Smp"
                --enum-class-locations "PrimitiveTypeKind:${ARG_MODULE}.Smp"
        COMMAND "${Python_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/cmake/postprocess_python_stubs.py"
                "${_stubs_dir}"
        COMMAND ${CMAKE_COMMAND} -E touch "${_stamp}"
        DEPENDS ${TARGET}
                "${CMAKE_CURRENT_SOURCE_DIR}/cmake/postprocess_python_stubs.py"
        COMMENT "Generating python type stubs for ${ARG_MODULE}"
        VERBATIM)

    add_custom_target(${TARGET}Stubs ALL DEPENDS "${_stamp}")
    set(${ARG_MODULE}_STUBS_DIR "${_stubs_dir}" PARENT_SCOPE)
endfunction()
