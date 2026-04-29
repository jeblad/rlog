cmake_minimum_required(VERSION 3.28)
project(rlog VERSION 0.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Git Versioning Logic ---
find_package(Git QUIET)
set(RLOG_FULL_VERSION "${PROJECT_VERSION}-0-unknown")
set(RLOG_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(RLOG_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(RLOG_VERSION_PATCH ${PROJECT_VERSION_PATCH})

if(Git_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --always --long --dirty
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_VERSION
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # Regex for å parse v1.2.3-4-gabc123[-dirty]
    # Match 1: Major, 2: Minor, 3: Patch, 4: Count, 5: Hash, 6: Dirty
    if(GIT_VERSION MATCHES "^v?([0-9]+)\\.([0-9]+)\\.([0-9]+)-([0-9]+)-(g[0-9a-f]+)((-dirty)?)$")
        set(RLOG_VERSION_MAJOR ${CMAKE_MATCH_1})
        set(RLOG_VERSION_MINOR ${CMAKE_MATCH_2})
        set(RLOG_VERSION_PATCH ${CMAKE_MATCH_3})
        set(RLOG_FULL_VERSION "${RLOG_VERSION_MAJOR}.${RLOG_VERSION_MINOR}.${RLOG_VERSION_PATCH}-${CMAKE_MATCH_4}-${CMAKE_MATCH_5}${CMAKE_MATCH_6}")
    endif()
endif()

# --- Git Hook Automation ---
if(Git_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    set(HOOK_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/scripts/pre-commit.sh")
    set(HOOK_DEST "${CMAKE_CURRENT_SOURCE_DIR}/.git/hooks/pre-commit")
    
    if(EXISTS "${HOOK_SOURCE}")
        if(UNIX)
            execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink "${HOOK_SOURCE}" "${HOOK_DEST}")
        else()
            execute_process(COMMAND ${CMAKE_COMMAND} -E copy "${HOOK_SOURCE}" "${HOOK_DEST}")
        endif()
        
        message(STATUS "Git pre-commit hook installed/updated.")
    endif()
endif()

# Generate the header file directly in the source include directory
configure_file(rlog.hpp.in "${CMAKE_CURRENT_SOURCE_DIR}/include/rlog/rlog.hpp" @ONLY)

# Header-only library target
add_library(rlog INTERFACE)
add_library(rlog::rlog ALIAS rlog)

target_include_directories(rlog INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Dependencies
find_package(Intl REQUIRED)
target_link_libraries(rlog INTERFACE ${Intl_LIBRARY})

# Tests
option(RLOG_BUILD_TESTS "Build rlog unit tests" ON)

if(RLOG_BUILD_TESTS)
    enable_testing()
    add_executable(test_rlog tests/test_rlog.cpp)
    target_link_libraries(test_rlog PRIVATE rlog)

    # Send versjonsinformasjon direkte fra CMake til testen som macros
    target_compile_definitions(test_rlog PRIVATE
        CMAKE_EXPECTED_VERSION_MAJOR=${RLOG_VERSION_MAJOR}
        CMAKE_EXPECTED_VERSION_MINOR=${RLOG_VERSION_MINOR}
        CMAKE_EXPECTED_VERSION_PATCH=${RLOG_VERSION_PATCH}
        CMAKE_EXPECTED_VERSION="${RLOG_FULL_VERSION}"
    )
    
    add_test(NAME rlog_unit_tests COMMAND test_rlog)
endif()

install(TARGETS rlog EXPORT rlogTargets)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/rlog/rlog.hpp" DESTINATION include/rlog)
install(EXPORT rlogTargets
    FILE rlogConfig.cmake
    DESTINATION lib/cmake/rlog)
