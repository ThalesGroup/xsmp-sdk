
if (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
    set(ENV_LIB_PATH PATH)
    set(ENV_SEPARATOR ";")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL Darwin)
    set(ENV_LIB_PATH DYLD_LIBRARY_PATH)
    set(ENV_SEPARATOR ":")
else()
    set(ENV_LIB_PATH LD_LIBRARY_PATH)
    set(ENV_SEPARATOR ":")
endif()
set(ENV_PATH PATH)
set(ENV_PYTHON_PATH PYTHONPATH)


function(_PathAppend variable path)
    if (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
        string(REPLACE "/" "\\\\" path "${path}")
    endif()
    if(DEFINED ENV{${variable}})
        set(ENV{${variable}} "$ENV{${variable}}${ENV_SEPARATOR}${path}")
    else()
        set(ENV{${variable}} "${path}")
    endif()
endfunction(_PathAppend)

function(_PathPrepend variable path)
    if (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
        string(REPLACE "/" "\\\\" path "${path}")
    endif()
    if(DEFINED ENV{${variable}})
        set(ENV{${variable}} "${path}${ENV_SEPARATOR}$ENV{${variable}}")
    else()
        set(ENV{${variable}} "${path}")
    endif()
endfunction(_PathPrepend)


function(PythonPathAppend path)
    _PathAppend(${ENV_PYTHON_PATH} ${path})
endfunction(PythonPathAppend)

function(PythonPathPrepend path)
    _PathPrepend(${ENV_PYTHON_PATH} ${path})
endfunction(PythonPathPrepend)

function(PathAppend path)
    _PathAppend(${ENV_PATH} ${path})
endfunction(PathAppend)

function(PathPrepend path)
    _PathPrepend(${ENV_PATH} ${path})
endfunction(PathPrepend)

function(LibraryPathAppend path)
    _PathAppend(${ENV_LIB_PATH} ${path})
endfunction(LibraryPathAppend)

function(LibraryPathPrepend path)
    _PathPrepend(${ENV_PYTHON_PATH} ${path})
endfunction(LibraryPathPrepend)