cmake_minimum_required(VERSION 3.19)

# Required for glm submodule (uses cmake_minimum_required with version < 3.5)
set(CMAKE_POLICY_VERSION_MINIMUM "3.5")

project(sampvoice)

set(CMAKE_CXX_STANDARD 17)

option(BUILD_CLIENT "Build the client plugin (SA:MP .asi, 32-bit only)" OFF)

# Architecture notes:
#   Server plugin (sampvoice-server) can be built as 32-bit or 64-bit.
#   PAWN cell size is always forced to 32-bit regardless of plugin architecture.
#   Client plugin (sampvoice-client) is SA:MP .asi, 32-bit only.
#
# Windows examples:
#   32-bit: cmake -B build32 -A Win32
#   64-bit: cmake -B build64 -A x64
# Linux examples:
#   32-bit: cmake -B build32 -DCMAKE_C_FLAGS=-m32 -DCMAKE_CXX_FLAGS=-m32
#   64-bit: cmake -B build64

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

function(GroupSourcesByFolder target)
    set(SOURCE_GROUP_DELIMITER "/")
    set(last_dir "")
    set(files "")

    get_target_property(sources ${target} SOURCES)

    foreach(file ${sources})
        file(RELATIVE_PATH relative_file "${CMAKE_CURRENT_SOURCE_DIR}" ${file})
        get_filename_component(dir "${relative_file}" PATH)
        if(NOT "${dir}" STREQUAL "${last_dir}")
            if(files)
                source_group("${last_dir}" FILES ${files})
            endif()
            set(files "")
        endif()
        set(files ${files} ${file})
        set(last_dir "${dir}")
    endforeach()

    if(files)
        source_group("${last_dir}" FILES ${files})
    endif()
endfunction()

add_subdirectory(lib/omp-sdk)
add_subdirectory(lib/omp-net)
add_subdirectory(server)

set_property(TARGET OMP-SDK PROPERTY FOLDER "Server")
set_property(TARGET OMP-Network PROPERTY FOLDER "Server")
set_property(TARGET sampvoice-server PROPERTY FOLDER "Server")

if(WIN32 AND BUILD_CLIENT)
    add_subdirectory(lib/sampapi)
    add_subdirectory(client)
    set_property(TARGET sampvoice-client PROPERTY FOLDER "Client")
endif()
