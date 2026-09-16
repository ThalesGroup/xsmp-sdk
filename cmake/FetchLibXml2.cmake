# Copyright 2026 THALES ALENIA SPACE FRANCE. All rights reserved.
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

include_guard(GLOBAL)

# Fetch a pinned, private libxml2. Deliberately do not use find_package() or a
# target supplied by a parent project: the Simulator must have the same small,
# static and reproducible XML implementation on every supported platform.
function(xsmp_fetch_libxml2)
    foreach(_target IN ITEMS LibXml2::LibXml2 LibXml2 libxml2::libxml2 libxml2 xml2)
        if(TARGET "${_target}")
            message(FATAL_ERROR
                    "xsmp-sdk embeds its own private libxml2 2.15.4, but target "
                    "'${_target}' already exists. Do not inject a system or parent "
                    "libxml2 target into xsmp-sdk.")
        endif()
    endforeach()

    include(FetchContent)

    # The dependency name includes the version to avoid accidentally accepting
    # a declaration made by a parent for an unrelated libxml2 release.
    set(_xsmp_libxml2_fetch_arguments
        URL      "https://download.gnome.org/sources/libxml2/2.15/libxml2-2.15.4.tar.xz"
        URL_HASH SHA256=98087fd181d9070724f3fbc65c7377db03038eb92bd882374daff44940138821)

    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.28")
        FetchContent_Declare(xsmp_libxml2_2_15_4
                             ${_xsmp_libxml2_fetch_arguments}
                             EXCLUDE_FROM_ALL)
    else()
        FetchContent_Declare(xsmp_libxml2_2_15_4
                             ${_xsmp_libxml2_fetch_arguments})
    endif()

    # Normal (non-cache) variables are intentionally scoped to this function.
    # With CMP0077 NEW in libxml2, they override both its defaults and any cache
    # options belonging to the parent without changing those parent options.
    set(BUILD_SHARED_LIBS OFF)
    set(LIBXML2_WITH_C14N OFF)
    set(LIBXML2_WITH_CATALOG OFF)
    set(LIBXML2_WITH_DEBUG OFF)
    set(LIBXML2_WITH_DOCS OFF)
    set(LIBXML2_WITH_FTP OFF)
    set(LIBXML2_WITH_HTML OFF)
    set(LIBXML2_WITH_HISTORY OFF)
    set(LIBXML2_WITH_HTTP OFF)
    set(LIBXML2_WITH_ICONV OFF)
    set(LIBXML2_WITH_ICU OFF)
    set(LIBXML2_WITH_ISO8859X OFF)
    set(LIBXML2_WITH_LEGACY OFF)
    set(LIBXML2_WITH_LZMA OFF)
    set(LIBXML2_WITH_MEM_DEBUG OFF)
    set(LIBXML2_WITH_MODULES OFF)
    set(LIBXML2_WITH_OUTPUT OFF)
    set(LIBXML2_WITH_PROGRAMS OFF)
    set(LIBXML2_WITH_PUSH OFF)
    set(LIBXML2_WITH_PYTHON OFF)
    set(LIBXML2_WITH_READLINE OFF)
    set(LIBXML2_WITH_READER OFF)
    set(LIBXML2_WITH_RELAXNG OFF)
    set(LIBXML2_WITH_SAX1 OFF)
    set(LIBXML2_WITH_SCHEMATRON OFF)
    set(LIBXML2_WITH_TESTS OFF)
    set(LIBXML2_WITH_THREAD_ALLOC OFF)
    set(LIBXML2_WITH_TLS OFF)
    set(LIBXML2_WITH_WRITER OFF)
    set(LIBXML2_WITH_XINCLUDE OFF)
    set(LIBXML2_WITH_XPATH OFF)
    set(LIBXML2_WITH_XPTR OFF)
    set(LIBXML2_WITH_ZLIB OFF)

    # EXCLUDE_FROM_ALL prevents libxml2's unconditional install() calls from
    # leaking its archive, headers, package files and documentation into an SDK
    # or wheel. FetchContent gained native support for that flag in CMake 3.28;
    # retain its documented expansion for the supported CMake 3.18--3.27 range.
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.28")
        FetchContent_MakeAvailable(xsmp_libxml2_2_15_4)
    else()
        FetchContent_GetProperties(xsmp_libxml2_2_15_4)
        if(NOT xsmp_libxml2_2_15_4_POPULATED)
            FetchContent_Populate(xsmp_libxml2_2_15_4)
            add_subdirectory("${xsmp_libxml2_2_15_4_SOURCE_DIR}"
                             "${xsmp_libxml2_2_15_4_BINARY_DIR}"
                             EXCLUDE_FROM_ALL)
        endif()
    endif()

    if(NOT TARGET LibXml2 OR NOT TARGET LibXml2::LibXml2)
        message(FATAL_ERROR "The bundled libxml2 did not define its expected targets.")
    endif()

    # GCC and Clang use this for both ELF and Mach-O. Symbols pulled from the
    # static archive consequently remain local to xsmp_simulator. On MSVC the
    # setting is ignored, while libxml2's static build publishes LIBXML_STATIC;
    # its declarations then carry no dllexport and CMake's DLL auto-export only
    # scans Simulator's own object files, not objects in linked archives.
    set_target_properties(LibXml2 PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${xsmp_libxml2_2_15_4_BINARY_DIR}"
        C_VISIBILITY_PRESET hidden
        EXCLUDE_FROM_ALL TRUE)
    foreach(_config IN LISTS CMAKE_CONFIGURATION_TYPES)
        string(TOUPPER "${_config}" _config_upper)
        set_target_properties(LibXml2 PROPERTIES
            "ARCHIVE_OUTPUT_DIRECTORY_${_config_upper}"
            "${xsmp_libxml2_2_15_4_BINARY_DIR}/${_config}")
    endforeach()

    set(_xsmp_libxml2_license_file
        "${xsmp_libxml2_2_15_4_SOURCE_DIR}/Copyright")
    if(NOT EXISTS "${_xsmp_libxml2_license_file}")
        message(FATAL_ERROR
                "The bundled libxml2 source does not contain its Copyright notice.")
    endif()
    set(XSMP_LIBXML2_LICENSE_FILE "${_xsmp_libxml2_license_file}" PARENT_SCOPE)
    message(STATUS "Using bundled private libxml2 2.15.4")
endfunction()
