
cmake_minimum_required(VERSION 3.14)


# Extract project version from source
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/include/Xsmp/version.h"
     xsmp_version_defines REGEX "#define XSMP_VERSION_(MAJOR|MINOR|PATCH) ")

foreach(ver ${xsmp_version_defines})
  if(ver MATCHES [[#define XSMP_VERSION_(MAJOR|MINOR|PATCH) +([^ ]+)$]])
    set(XSMP_VERSION_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}")
  endif()
endforeach()

if(XSMP_VERSION_PATCH MATCHES [[\.([a-zA-Z0-9]+)$]])
  set(xsmp-sdk_VERSION_TYPE "${CMAKE_MATCH_1}")
endif()
string(REGEX MATCH "^[0-9]+" XSMP_VERSION_PATCH "${XSMP_VERSION_PATCH}")


project(xsmp-sdk
    VERSION "${XSMP_VERSION_MAJOR}.${XSMP_VERSION_MINOR}.${XSMP_VERSION_PATCH}"
    DESCRIPTION "XSMP Simulator Development Kit."
    HOMEPAGE_URL https://github.com/ThalesGroup/xsmp-sdk
    LANGUAGES CXX)

message(STATUS "xsmp-sdk v${xsmp-sdk_VERSION} ${xsmp-sdk_VERSION_TYPE}")



if( CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR )
    set(XSMP_MASTER_PROJECT ON)
else ()
    set(XSMP_MASTER_PROJECT OFF)
endif ()

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
    cmake_policy(SET CMP0135 NEW)
endif()

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.27.0")
    cmake_policy(SET CMP0144 NEW)
endif()

# Initialize some default paths
include(GNUInstallDirs)

include(FetchContent)

FetchContent_Declare(
    ecss_smp
    URL      https://ecss.nl/wp-content/uploads/2020/04/ECSS_SMP_Issue1%282March2020%29.zip
    URL_HASH MD5=973D4CA21FDAC204A4419CD06CD65185
)

FetchContent_Declare(
    date_src
    GIT_REPOSITORY https://github.com/HowardHinnant/date.git
    GIT_TAG        v3.0.1
)

FetchContent_MakeAvailable(ecss_smp date_src)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(XSMP_BUILD_TESTS           "whether or not tests should be built"            ${XSMP_MASTER_PROJECT})
option(XSMP_BUILD_PYTHON_BINDINGS "whether or not Python bindings should be built"  ON)

option(XSMP_BUILD_PACKAGE         "whether or not the package should be built"      ${XSMP_MASTER_PROJECT})
option(XSMP_BUILD_EXAMPLES        "whether or not examples should be built"         OFF)
option(XSMP_ENABLE_INSTALL        "whether or not to enable the install rule"       ${XSMP_MASTER_PROJECT})
option(XSMP_ENABLE_CODECOVERAGE   "Enable code coverage testing support"            OFF)
option(XSMP_BUILD_WITH_WARNINGS   "Enable all compiler warnings"                    OFF)


set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if(WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" CACHE PATH "Archive output dir.")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" CACHE PATH "Library output dir.")
else()
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" CACHE PATH "Archive output dir.")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" CACHE PATH "Library output dir.")
endif(WIN32)

set(CMAKE_PDB_OUTPUT_DIRECTORY     "${CMAKE_BINARY_DIR}/bin" CACHE PATH "PDB (MSVC debug symbol)output dir.")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" CACHE PATH "Executable/dll output dir.")

if(APPLE)
    set(CMAKE_MACOSX_RPATH TRUE)
    list(APPEND CMAKE_INSTALL_RPATH "@loader_path/" "@executable_path/")
elseif(UNIX)
    list(APPEND CMAKE_INSTALL_RPATH "$ORIGIN")
endif()


if(XSMP_ENABLE_CODECOVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "(Apple)?[Cc]lang" )
      add_compile_options(--coverage)
      add_link_options(--coverage)
      set(GCOVR_OPTIONS --gcov-executable "llvm-cov gcov")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
      add_compile_options(--coverage)
      add_link_options(--coverage)
      set(GCOVR_OPTIONS)
    else()
        message( ERROR "Code coverage is not supported for ${CMAKE_CXX_COMPILER_ID}" )
    endif()
endif(XSMP_ENABLE_CODECOVERAGE)

#disable inheritance via dominance warning on MSVC
if(MSVC)
    add_compile_options(/wd4250)
endif(MSVC)

if(XSMP_BUILD_WITH_WARNINGS)
    macro(add_compiler_flags)
        foreach(flag ${ARGV})
            set(XSMP_COMPILE_OPTIONS ${XSMP_COMPILE_OPTIONS} ${flag})
        endforeach()
    endmacro()
    
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    
        # The following options are not valid when clang-cl is used.
        if(NOT MSVC)
            add_compiler_flags(-pedantic)
        endif()
    endif()
    
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        add_compiler_flags(-Wall)
        add_compiler_flags(-Wextra)
        add_compiler_flags(-fdiagnostics-show-option)
        add_compiler_flags(-Wconversion)
        add_compiler_flags(-Wold-style-cast)
        add_compiler_flags(-Wfloat-equal)
        add_compiler_flags(-Wlogical-op)
        add_compiler_flags(-Wundef)
        add_compiler_flags(-Wredundant-decls)
        add_compiler_flags(-Wwrite-strings)
        add_compiler_flags(-Wpointer-arith)
        add_compiler_flags(-Wcast-qual)
        add_compiler_flags(-Wformat=2)
        add_compiler_flags(-Wmissing-include-dirs)
        add_compiler_flags(-Wcast-align)
        add_compiler_flags(-Wno-non-virtual-dtor)
        add_compiler_flags(-Wctor-dtor-privacy)
        add_compiler_flags(-Wsign-conversion)
        add_compiler_flags(-Wdisabled-optimization)
        add_compiler_flags(-Weffc++)
        add_compiler_flags(-Winvalid-pch)
        add_compiler_flags(-Woverloaded-virtual)
        add_compiler_flags(-Wunused-but-set-variable)
        add_compiler_flags(-Wunused-result)
        add_compiler_flags(-Wnoexcept)
        add_compiler_flags(-Wno-missing-field-initializers)
        add_compiler_flags(-Wdouble-promotion)
        add_compiler_flags(-Wtrampolines)
        add_compiler_flags(-Wzero-as-null-pointer-constant)
        add_compiler_flags(-Wuseless-cast)
        add_compiler_flags(-Wvector-operation-performance)
        add_compiler_flags(-Wshift-overflow=2)
        add_compiler_flags(-Wnull-dereference)
        add_compiler_flags(-Wduplicated-cond)

        if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.0)
            add_compiler_flags(-Walloc-zero)
            add_compiler_flags(-Walloca)
            add_compiler_flags(-Wduplicated-branches)
        endif()

        if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8.0)
            add_compiler_flags(-Wcast-align=strict)
            add_compiler_flags(-Wcatch-value)
        endif()
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_compiler_flags(-Weverything)
        add_compiler_flags(-Wno-c++98-compat)
        add_compiler_flags(-Wno-c++98-compat-pedantic)
        add_compiler_flags(-Wno-c++98-compat-bind-to-temporary-copy)
        add_compiler_flags(-Wno-c++98-compat-local-type-template-args)
        add_compiler_flags(-Qunused-arguments -fcolor-diagnostics) # needed for ccache integration
    endif()

    if(MSVC)
        add_compiler_flags(/std:c++latest)
        add_compiler_flags(/permissive-)
        add_compiler_flags(/W4)
        add_compiler_flags(
            /wd4514
            /wd4571
            /wd5264
            /wd4710
            /wd4711
            /wd4616
            /wd4619
            /wd4250
        )
    endif(MSVC)
endif(XSMP_BUILD_WITH_WARNINGS)

# --------------------------------------------------------------------
# Create Smp interface library
# --------------------------------------------------------------------
add_library(Smp INTERFACE)
add_library(Xsmp::Smp ALIAS Smp)


# Smp/Uuid.h requires constexpr const_reference std::array::operator[](size_type) which is available since C++14 otherwise C++11 would be enough
target_compile_features(Smp INTERFACE cxx_std_14)


target_include_directories(
    Smp
    INTERFACE $<BUILD_INTERFACE:${ecss_smp_SOURCE_DIR}>
              $<INSTALL_INTERFACE:include>
)

# --------------------------------------------------------------------
# Create the LibraryHelper Object
# --------------------------------------------------------------------
add_library(LibraryHelper OBJECT 
    src/Xsmp/LibraryHelper.cpp
)
add_library(Xsmp::LibraryHelper ALIAS LibraryHelper)
target_include_directories(
    LibraryHelper
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(LibraryHelper INTERFACE ${CMAKE_DL_LIBS})

# --------------------------------------------------------------------
# Create Cdk library
# --------------------------------------------------------------------

add_library(Cdk SHARED 
    src/Smp/AnySimple.cpp
    src/Smp/DummySymbols.cpp
    src/Smp/StreamOperators.cpp
    src/Smp/Uuid.cpp
    src/Xsmp/Aggregate.cpp
    src/Xsmp/Component.cpp
    src/Xsmp/Composite.cpp
    src/Xsmp/Container.cpp
    src/Xsmp/DateTime.cpp
    src/Xsmp/Duration.cpp
    src/Xsmp/EntryPoint.cpp
    src/Xsmp/EntryPointPublisher.cpp
    src/Xsmp/EventConsumer.cpp
    src/Xsmp/EventProvider.cpp
    src/Xsmp/EventSink.cpp
    src/Xsmp/EventSource.cpp
    src/Xsmp/Exception.cpp
    src/Xsmp/Factory.cpp
    src/Xsmp/FallibleModel.cpp
    src/Xsmp/Field.cpp
    src/Xsmp/Helper.cpp
    src/Xsmp/Object.cpp
    src/Xsmp/Reference.cpp
    src/Xsmp/Request.cpp
)
set_target_properties(Cdk PROPERTIES OUTPUT_NAME "xsmp_cdk")

target_link_libraries(Cdk PUBLIC Xsmp::Smp PRIVATE date::date)


add_library(Xsmp::Cdk ALIAS Cdk)
set(COMPILE_WITH_C_LOCALE ON)
target_compile_options(Cdk PRIVATE ${XSMP_COMPILE_OPTIONS})

target_compile_features(Cdk PUBLIC cxx_std_17)

target_include_directories(
    Cdk
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
           $<INSTALL_INTERFACE:include>
    PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)


# --------------------------------------------------------------------
# Create Python bindings
# --------------------------------------------------------------------
if(XSMP_BUILD_PYTHON_BINDINGS)
    find_package(Python COMPONENTS Interpreter Development)
    FetchContent_Declare(
      pybind11
      GIT_REPOSITORY https://github.com/pybind/pybind11.git
      GIT_TAG        v2.12.0
    )
    FetchContent_MakeAvailable(pybind11)


    pybind11_add_module(Python src/python/ecss_smp.cpp)
    set_target_properties(Python PROPERTIES OUTPUT_NAME "ecss_smp")

    if(APPLE)
        set_target_properties(Python PROPERTIES INSTALL_RPATH "@loader_path/${CMAKE_INSTALL_LIBDIR}")
    elseif(UNIX)
        set_target_properties(Python PROPERTIES INSTALL_RPATH "$ORIGIN/${CMAKE_INSTALL_LIBDIR}")
    endif()

    add_library(Xsmp::Python ALIAS Python)
    target_include_directories(
        Python
        PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    )
    target_link_libraries(Python PRIVATE Xsmp::Cdk Xsmp::LibraryHelper)
    target_compile_definitions(Python PRIVATE VERSION_INFO=${PROJECT_VERSION})
    target_compile_options(Python PRIVATE ${XSMP_COMPILE_OPTIONS})
    if(SKBUILD STREQUAL "2")
        install(TARGETS Python DESTINATION .)
    else()
        install(TARGETS Python DESTINATION ${CMAKE_INSTALL_PREFIX}${Python_SITELIB}/)
    endif()
endif(XSMP_BUILD_PYTHON_BINDINGS)



# --------------------------------------------------------------------
# Create EventManager library
# --------------------------------------------------------------------
add_library(EventManager SHARED 
    src-gen/Xsmp/Services/XsmpEventManagerGen.cpp
    src-gen/xsmp_event_manager.cpp
    src-gen/xsmp_event_manager.pkg.cpp
    src/Xsmp/Services/XsmpEventManager.cpp
)
set_target_properties(EventManager PROPERTIES OUTPUT_NAME "xsmp_event_manager")
add_library(Xsmp::EventManager ALIAS EventManager)

target_include_directories(
    EventManager
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src> 
           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src-gen>
)

target_link_libraries(EventManager PUBLIC Xsmp::Cdk)

target_compile_options(EventManager PRIVATE ${XSMP_COMPILE_OPTIONS})


# --------------------------------------------------------------------
# Create LinkRegistry library
# --------------------------------------------------------------------
add_library(LinkRegistry SHARED 
    src-gen/Xsmp/Services/XsmpLinkRegistryGen.cpp
    src-gen/xsmp_link_registry.cpp
    src-gen/xsmp_link_registry.pkg.cpp
    src/Xsmp/Services/XsmpLinkRegistry.cpp
)

set_target_properties(LinkRegistry PROPERTIES OUTPUT_NAME "xsmp_link_registry")
add_library(Xsmp::LinkRegistry ALIAS LinkRegistry)

target_include_directories(
    LinkRegistry
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src> 
           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src-gen>
)

target_link_libraries(LinkRegistry PUBLIC Xsmp::Cdk)

target_compile_options(LinkRegistry PRIVATE ${XSMP_COMPILE_OPTIONS})


# --------------------------------------------------------------------
# Create Logger library
# --------------------------------------------------------------------
add_library(Logger SHARED 
    src-gen/Xsmp/Services/XsmpLoggerGen.cpp
    src-gen/xsmp_logger.cpp
    src-gen/xsmp_logger.pkg.cpp
    src/Xsmp/Services/XsmpLogger.cpp
)
set_target_properties(Logger PROPERTIES OUTPUT_NAME "xsmp_logger")
add_library(Xsmp::Logger ALIAS Logger)

target_include_directories(
    Logger
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src> 
           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src-gen>
)

target_link_libraries(Logger PUBLIC Xsmp::Cdk)

target_compile_options(Logger PRIVATE ${XSMP_COMPILE_OPTIONS})


# --------------------------------------------------------------------
# Create Resolver library
# --------------------------------------------------------------------
add_library(Resolver SHARED 
    src-gen/Xsmp/Services/XsmpResolverGen.cpp
    src-gen/xsmp_resolver.cpp
    src-gen/xsmp_resolver.pkg.cpp
    src/Xsmp/Services/XsmpResolver.cpp
)

set_target_properties(Resolver PROPERTIES OUTPUT_NAME "xsmp_resolver")
add_library(Xsmp::Resolver ALIAS Resolver)

target_include_directories(
    Resolver
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src> 
           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src-gen>
)

target_link_libraries(Resolver PUBLIC Xsmp::Cdk)
target_compile_options(Resolver PRIVATE ${XSMP_COMPILE_OPTIONS})

# --------------------------------------------------------------------
# Create Scheduler library
# --------------------------------------------------------------------
add_library(Scheduler SHARED 
    src-gen/Xsmp/Services/XsmpSchedulerGen.cpp
    src-gen/xsmp_scheduler.cpp
    src-gen/xsmp_scheduler.pkg.cpp
    src/Xsmp/Services/XsmpScheduler.cpp
)

set_target_properties(Scheduler PROPERTIES OUTPUT_NAME "xsmp_scheduler")
add_library(Xsmp::Scheduler ALIAS Scheduler)

target_include_directories(
    Scheduler
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src> 
           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src-gen>
)

target_link_libraries(Scheduler PUBLIC Xsmp::Cdk)
target_compile_options(Scheduler PRIVATE ${XSMP_COMPILE_OPTIONS})


# --------------------------------------------------------------------
# Create TimeKeeper library
# --------------------------------------------------------------------
add_library(TimeKeeper SHARED 
    src-gen/Xsmp/Services/XsmpTimeKeeperGen.cpp
    src-gen/xsmp_time_keeper.cpp
    src-gen/xsmp_time_keeper.pkg.cpp
    src/Xsmp/Services/XsmpTimeKeeper.cpp
)

set_target_properties(TimeKeeper PROPERTIES OUTPUT_NAME "xsmp_time_keeper")
add_library(Xsmp::TimeKeeper ALIAS TimeKeeper)

target_include_directories(
    TimeKeeper
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src> 
           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src-gen>
)

target_link_libraries(TimeKeeper PUBLIC Xsmp::Cdk)
target_compile_options(TimeKeeper PRIVATE ${XSMP_COMPILE_OPTIONS})


# --------------------------------------------------------------------
# Create Bundle Services library
# --------------------------------------------------------------------
add_library(Services SHARED 
    src/xsmp_services.cpp
    src/xsmp_services.pkg.cpp
)

set_target_properties(Services PROPERTIES OUTPUT_NAME "xsmp_services")
add_library(Xsmp::Services ALIAS Services)

target_include_directories(
    Services
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

target_link_libraries(Services PUBLIC Xsmp::Logger 
                                      Xsmp::Resolver
                                      Xsmp::Scheduler
                                      Xsmp::EventManager 
                                      Xsmp::LinkRegistry
                                      Xsmp::TimeKeeper)
target_compile_options(Services PRIVATE ${XSMP_COMPILE_OPTIONS})


# --------------------------------------------------------------------
# Create Simulator library
# --------------------------------------------------------------------
add_library(Simulator SHARED 
    src/Xsmp/FactoryCollection.cpp
    src/Xsmp/Publication/Field.cpp
    src/Xsmp/Publication/Operation.cpp
    src/Xsmp/Publication/Property.cpp
    src/Xsmp/Publication/Publication.cpp
    src/Xsmp/Publication/Request.cpp
    src/Xsmp/Publication/TypeRegistry.cpp
    src/Xsmp/Publication/Type.cpp
    src/Xsmp/Simulator.cpp
    src/Xsmp/StorageReader.cpp
    src/Xsmp/StorageWriter.cpp
)

set_target_properties(Simulator PROPERTIES OUTPUT_NAME "xsmp_simulator")
add_library(Xsmp::Simulator ALIAS Simulator)

target_include_directories(
    Simulator
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

target_link_libraries(Simulator PUBLIC Xsmp::Cdk Xsmp::LibraryHelper)

target_compile_options(Simulator PRIVATE ${XSMP_COMPILE_OPTIONS})

if((CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0) OR
   (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 11))
    target_link_libraries(Simulator PUBLIC stdc++fs)
endif()





if(XSMP_BUILD_PACKAGE)
    set(package_files include/ src/ src-gen/ CMakeLists.txt LICENSE)
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/xsmp-sdk-src.zip
        COMMAND ${CMAKE_COMMAND} -E tar c ${CMAKE_CURRENT_BINARY_DIR}/xsmp-sdk-src.zip --format=zip -- ${package_files}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${package_files})
    add_custom_target(package DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/xsmp-sdk-src.zip)
endif(XSMP_BUILD_PACKAGE)




if(XSMP_BUILD_TESTS)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        e1a38bc3707741d249fa22d2064552a08e37555b
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "")
    set(INSTALL_GTEST OFF CACHE BOOL "")
    FetchContent_MakeAvailable(googletest)

    include(GoogleTest)

    file(GLOB_RECURSE SRC-TEST-PKG CONFIGURE_DEPENDS src-gen/xsmp_tests.cpp src-gen/xsmp_tests.pkg.cpp src/Xsmp/Tests/*.cpp src-gen/Xsmp/Tests/*.cpp)

    add_library(TestsPackage SHARED ${SRC-TEST-PKG})

    set_target_properties(TestsPackage PROPERTIES OUTPUT_NAME "xsmp_tests")
    add_library(Xsmp::TestsPackage ALIAS TestsPackage)
    target_include_directories(
        TestsPackage
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src> 
               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src-gen>
    )
    target_link_libraries(TestsPackage PUBLIC Xsmp::Cdk)
    target_compile_options(TestsPackage PRIVATE ${XSMP_COMPILE_OPTIONS})
    install(TARGETS TestsPackage)
    include(CTest)

    file(GLOB_RECURSE SRC-TEST CONFIGURE_DEPENDS tests/*.cpp)
    add_executable (XsmpTests ${SRC-TEST})
    add_executable (Xsmp::XsmpTests ALIAS XsmpTests)

    target_include_directories(XsmpTests PRIVATE tests)
    target_link_libraries(XsmpTests PUBLIC
                                    Xsmp::Cdk
                                    Xsmp::TestsPackage
                                    Xsmp::Simulator
                                    Xsmp::Services
                                    GTest::gtest_main
    )
    #disable some warnings on MSVC&Clang with GTest
    if(MSVC)
        target_compile_options(XsmpTests PRIVATE /bigobj /wd4250)
    endif(MSVC)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(XsmpTests PRIVATE -Wno-unused-value)
    endif()
    
    gtest_discover_tests(XsmpTests)
    include(Pytest)
    pytest_discover_tests() 
endif(XSMP_BUILD_TESTS)

if(XSMP_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif(XSMP_BUILD_EXAMPLES)

if(XSMP_ENABLE_CODECOVERAGE)
    include(CTest)
    find_program(GCOVR_PATH gcovr REQUIRED)
    add_custom_target(coverage
        COMMAND rm -rf $$\(find . -name \"*.gcda\"\)
        # run tests
        COMMAND ${CMAKE_CTEST_COMMAND} -T Test --output-on-failure #-R PublicationTest
        # generate report
        COMMAND mkdir -p ${CMAKE_BINARY_DIR}/report
        COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ${GCOVR_PATH} --root . --html-nested ${CMAKE_BINARY_DIR}/report/index.html --html-title \"xsmp-sdk Code Coverage Report\"
            -f \"include/\" -f \"src/\"
            -e \"src/python/\" -e \"src/Xsmp/Tests/\"
            --delete --sort-uncovered --print-summary --calls --decisions ${GCOVR_OPTIONS}
            ${CMAKE_BINARY_DIR}
        COMMAND echo Generated report: ${CMAKE_BINARY_DIR}/report/index.html
        DEPENDS XsmpTests
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif(XSMP_ENABLE_CODECOVERAGE)

if(XSMP_ENABLE_INSTALL)
    include(CMakePackageConfigHelpers)
    include(GNUInstallDirs)
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/cmake"
            DESTINATION "lib/"
            FILES_MATCHING
            PATTERN "*.cmake"
    )
    install(DIRECTORY "include/" "${ecss_smp_SOURCE_DIR}/Smp"
            DESTINATION "include"
            FILES_MATCHING
            PATTERN "*.h"
    )
    if(NOT SKBUILD STREQUAL "2")
        install(DIRECTORY "python/xsmp"
                          "python/xsmp_event_manager"
                          "python/xsmp_link_registry"
                          "python/xsmp_logger"
                          "python/xsmp_resolver"
                          "python/xsmp_scheduler"
                          "python/xsmp_time_keeper"
                DESTINATION "${CMAKE_INSTALL_PREFIX}${Python_SITELIB}/"
                FILES_MATCHING
                PATTERN "*.py"
        )
    endif()

    install(TARGETS Smp Cdk LibraryHelper Simulator EventManager LinkRegistry Logger Resolver Scheduler TimeKeeper
        EXPORT ${PROJECT_NAME}-config
    )
    install(EXPORT ${PROJECT_NAME}-config
        NAMESPACE Xsmp::
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
    )
endif(XSMP_ENABLE_INSTALL)
