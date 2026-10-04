cmake_minimum_required(VERSION 3.21)

project(aquiLua
    VERSION 0.1.0
    DESCRIPTION "Hot-reloadable Luau scripting"
    LANGUAGES CXX
)

option(AQUILUA_BUILD_SANDBOX
    "Build the sandbox executable (sandbox/); off by default when aquiLua is a subproject"
    ${PROJECT_IS_TOP_LEVEL})

if(PROJECT_IS_TOP_LEVEL)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/external/luau/CMakeLists.txt")
    message(FATAL_ERROR
        "aquiLua: external/luau is missing. It is a git submodule; run: git submodule update --init")
endif()

set(LUAU_BUILD_CLI OFF CACHE BOOL "Build Luau CLI")
set(LUAU_BUILD_TESTS OFF CACHE BOOL "Build Luau tests")
add_subdirectory(external/luau EXCLUDE_FROM_ALL)

file(GLOB_RECURSE AQUILUA_SOURCES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE AQUILUA_HEADERS CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/include/*.hpp"
    "${PROJECT_SOURCE_DIR}/include/*.h"
    "${PROJECT_SOURCE_DIR}/src/*.hpp"
    "${PROJECT_SOURCE_DIR}/src/*.h")

set(AQUILUA_WARNINGS
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>)

if(AQUILUA_SOURCES)
    add_library(aquiLua STATIC ${AQUILUA_SOURCES} ${AQUILUA_HEADERS})
    target_include_directories(aquiLua PRIVATE "${PROJECT_SOURCE_DIR}/src")
    target_compile_options(aquiLua PRIVATE ${AQUILUA_WARNINGS})
    set(_aquilua_usage PUBLIC)
else()
    add_library(aquiLua INTERFACE ${AQUILUA_HEADERS})
    set(_aquilua_usage INTERFACE)
endif()
add_library(aquiLua::aquiLua ALIAS aquiLua)

target_compile_features(aquiLua ${_aquilua_usage} cxx_std_20)
target_include_directories(aquiLua ${_aquilua_usage} "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(aquiLua ${_aquilua_usage} Luau.VM Luau.Compiler Luau.Analysis)

if(AQUILUA_BUILD_SANDBOX)
    file(GLOB_RECURSE AQUILUA_SANDBOX_SOURCES CONFIGURE_DEPENDS
        "${PROJECT_SOURCE_DIR}/sandbox/*.cpp")
    add_executable(aquiLua_sandbox ${AQUILUA_SANDBOX_SOURCES})
    target_link_libraries(aquiLua_sandbox PRIVATE aquiLua::aquiLua)
    target_include_directories(aquiLua_sandbox PRIVATE "${PROJECT_SOURCE_DIR}/src")
    target_compile_definitions(aquiLua_sandbox PRIVATE
        AQUILUA_ROOT_DIR="${PROJECT_SOURCE_DIR}")
    target_compile_options(aquiLua_sandbox PRIVATE ${AQUILUA_WARNINGS})
endif()
