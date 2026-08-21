set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE sampvoice_client_source_list
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

set(SAMPAPI_ROOT "${PROJECT_SOURCE_DIR}/lib/sampapi")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libraries)

# Locate d3dx9 headers (DXSDK_DIR, NuGet Microsoft.DXSDK.D3DX, or vcpkg)
if(DEFINED ENV{DXSDK_DIR} AND EXISTS "$ENV{DXSDK_DIR}/Include/d3dx9.h")
    include_directories("$ENV{DXSDK_DIR}/Include")
    find_library(D3DX9_LIB d3dx9.lib PATHS "$ENV{DXSDK_DIR}/Lib/x86" NO_DEFAULT_PATH)
    message(STATUS "Found d3dx9 via DXSDK_DIR: $ENV{DXSDK_DIR}")
else()
    file(GLOB D3DX9_NUGET_DIRS
        "${CMAKE_BINARY_DIR}/../Microsoft.DXSDK.D3DX.*/build/native"
        "${PROJECT_SOURCE_DIR}/../Microsoft.DXSDK.D3DX.*/build/native"
    )
    list(SORT D3DX9_NUGET_DIRS)
    list(REVERSE D3DX9_NUGET_DIRS)
    if(D3DX9_NUGET_DIRS)
        list(GET D3DX9_NUGET_DIRS 0 D3DX9_NUGET_DIR)
        include_directories("${D3DX9_NUGET_DIR}/include")
        find_library(D3DX9_LIB d3dx9.lib
            PATHS "${D3DX9_NUGET_DIR}/release/lib/x86"
            NO_DEFAULT_PATH
        )
        if(D3DX9_LIB)
            message(STATUS "Found d3dx9 via NuGet: ${D3DX9_NUGET_DIR} (lib: ${D3DX9_LIB})")
        else()
            message(WARNING "d3dx9.lib not found in NuGet package at ${D3DX9_NUGET_DIR}")
        endif()
    else()
        message(WARNING "d3dx9.h not found. Set DXSDK_DIR or 'nuget install Microsoft.DXSDK.D3DX'")
    endif()
endif()

add_definitions(-DSAMP_EN)

# Version parameters (parallel lists)
set(VER_DIRS  "0.3.7-R1"  "0.3.7-R3-1"  "0.3.7-R5-1"  "0.3.DL-1")
set(VER_NS    "v037r1"    "v037r3"      "v037r5"      "v03dl")
set(VER_SUFFIX "r1"       "r3"           "r5"          "dl")

list(LENGTH VER_DIRS NUM_VER)
math(EXPR NUM_VER "${NUM_VER} - 1")

foreach(i RANGE ${NUM_VER})
    list(GET VER_DIRS ${i} ver_dir)
    list(GET VER_NS ${i} ver_ns)
    list(GET VER_SUFFIX ${i} ver_suffix)

    add_library(sampvoice_${ver_suffix} SHARED EXCLUDE_FROM_ALL
        ${sampvoice_client_source_list}
    )

    target_include_directories(sampvoice_${ver_suffix} PRIVATE
        "${SAMPAPI_ROOT}/include/sampapi/${ver_dir}"
        "${SAMPAPI_ROOT}/include"
    )

    string(TOUPPER ${ver_suffix} VER_UPPER)
    target_compile_definitions(sampvoice_${ver_suffix} PRIVATE
        SAMPVOICE_NS=${ver_ns}
        SAMP_${VER_UPPER}=1
    )

    target_link_libraries(sampvoice_${ver_suffix} PRIVATE
        sampapi
        ${D3DX9_LIB}
        bass_fx.lib
        bass.lib
        opus.lib
        dxguid.lib
    )

    set_target_properties(sampvoice_${ver_suffix} PROPERTIES
        PREFIX ""
        OUTPUT_NAME "sampvoice_${ver_suffix}"
        SUFFIX ".asi"
    )

    set_property(TARGET sampvoice_${ver_suffix} PROPERTY FOLDER "Client")

    if(MSVC)
        set_target_properties(sampvoice_${ver_suffix} PROPERTIES
            LINK_FLAGS "/SAFESEH:NO"
        )
        # Store string literals as UTF-8 so svlog.txt is consistently UTF-8.
        target_compile_options(sampvoice_${ver_suffix} PRIVATE "/utf-8")
    endif()
endforeach()

add_custom_target(sampvoice-client
    COMMAND ${CMAKE_COMMAND} --build . --target sampvoice_r1
    COMMAND ${CMAKE_COMMAND} --build . --target sampvoice_r3
    COMMAND ${CMAKE_COMMAND} --build . --target sampvoice_r5
    COMMAND ${CMAKE_COMMAND} --build . --target sampvoice_dl
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

if(NUM_VER GREATER_EQUAL 0)
    GroupSourcesByFolder(sampvoice_${VER_SUFFIX})
endif()
