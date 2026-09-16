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

function(_xsmp_copy_schema source destination)
    if(NOT EXISTS "${source}")
        message(FATAL_ERROR "Required SMP schema not found: ${source}")
    endif()
    configure_file("${source}" "${destination}" COPYONLY)
endfunction()

function(_xsmp_replace_in_schema file search replacement)
    file(READ "${file}" _contents)
    string(FIND "${_contents}" "${search}" _position)
    if(_position EQUAL -1)
        message(FATAL_ERROR
                "Expected text '${search}' was not found in schema ${file}")
    endif()
    string(REPLACE "${search}" "${replacement}" _contents "${_contents}")
    file(WRITE "${file}" "${_contents}")
endfunction()

# Prepare self-contained schema directories in the build tree. The Level 2
# schemas published in 2025 reference the 2019 Core namespaces, which are only
# distributed in the Level 1 Issue 1 archive. Nothing from those archives is
# copied into the source tree.
function(xsmp_prepare_smp_schemas output_directory l1_source l2_source legacy_source)
    set(_l1_directory "${output_directory}/l1")
    set(_l1_legacy_directory "${_l1_directory}/2019")
    set(_l2_directory "${output_directory}/l2")
    file(MAKE_DIRECTORY
         "${_l1_directory}"
         "${_l1_legacy_directory}/Core"
         "${_l1_legacy_directory}/Smdl"
         "${_l2_directory}")

    foreach(_schema
            Catalogue.xsd
            Configuration.xsd
            Elements.xsd
            Package.xsd
            Types.xsd
            xlink.xsd
            xml.xsd)
        _xsmp_copy_schema("${l1_source}/xml/Smdl/${_schema}"
                          "${_l1_directory}/${_schema}")
    endforeach()

    # Keep the previous Level 1 Configuration schema available for loading
    # existing 2019 artefacts. Its original directory layout is preserved so
    # that only the remote xml.xsd import needs to be relocated.
    _xsmp_copy_schema("${legacy_source}/XML/Smdl/Configuration.xsd"
                      "${_l1_legacy_directory}/Smdl/Configuration.xsd")
    _xsmp_copy_schema("${legacy_source}/XML/Core/Elements.xsd"
                      "${_l1_legacy_directory}/Core/Elements.xsd")
    _xsmp_copy_schema("${legacy_source}/XML/Core/Types.xsd"
                      "${_l1_legacy_directory}/Core/Types.xsd")
    _xsmp_copy_schema("${legacy_source}/XML/xlink.xsd"
                      "${_l1_legacy_directory}/xlink.xsd")
    _xsmp_copy_schema("${l1_source}/xml/Smdl/xml.xsd"
                      "${_l1_legacy_directory}/xml.xsd")
    _xsmp_replace_in_schema("${_l1_legacy_directory}/xlink.xsd"
                            "http://www.w3.org/2001/xml.xsd" "xml.xsd")

    foreach(_schema Assembly.xsd LinkBase.xsd Schedule.xsd)
        _xsmp_copy_schema("${l2_source}/xml/Smdl/${_schema}"
                          "${_l2_directory}/${_schema}")
    endforeach()
    _xsmp_copy_schema("${legacy_source}/XML/Core/Elements.xsd"
                      "${_l2_directory}/Elements.xsd")
    _xsmp_copy_schema("${legacy_source}/XML/Core/Types.xsd"
                      "${_l2_directory}/Types.xsd")
    _xsmp_copy_schema("${legacy_source}/XML/xlink.xsd"
                      "${_l2_directory}/xlink.xsd")
    _xsmp_copy_schema("${l1_source}/xml/Smdl/xml.xsd"
                      "${_l2_directory}/xml.xsd")

    # Make every import local to the flattened, sandboxable bundle.
    _xsmp_replace_in_schema("${_l2_directory}/Elements.xsd"
                            "../xlink.xsd" "xlink.xsd")
    _xsmp_replace_in_schema("${_l2_directory}/Types.xsd"
                            "../xlink.xsd" "xlink.xsd")
    _xsmp_replace_in_schema("${_l2_directory}/Types.xsd"
                            "../Core/Elements.xsd" "Elements.xsd")
    _xsmp_replace_in_schema("${_l2_directory}/xlink.xsd"
                            "http://www.w3.org/2001/xml.xsd" "xml.xsd")

    # The published Level 2 Schedule schema refers to a type which does not
    # exist in the referenced 2019 Core schema. xsd:duration is its standard
    # XML Schema equivalent (and is the correction used by xsmp-modeler).
    _xsmp_replace_in_schema("${_l2_directory}/Schedule.xsd"
                            "Elements:DayTimeDuration" "xsd:duration")

    set(XSMP_L1_SCHEMA_DIRECTORY "${_l1_directory}" PARENT_SCOPE)
    set(XSMP_L2_SCHEMA_DIRECTORY "${_l2_directory}" PARENT_SCOPE)
endfunction()
