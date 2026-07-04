cmake_minimum_required(VERSION 3.28)

# Read version from package.json
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/package.json" PACKAGE_JSON_CONTENT)
string(JSON PROJECT_VERSION_FROM_PACKAGE_JSON GET "${PACKAGE_JSON_CONTENT}" "version")

project(rlog VERSION ${PROJECT_VERSION_FROM_PACKAGE_JSON} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Git Versioning Logic ---
find_package(Git QUIET)
set(RLOG_FULL_VERSION "v${PROJECT_VERSION}")
set(RLOG_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(RLOG_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(RLOG_VERSION_PATCH ${PROJECT_VERSION_PATCH})

if(Git_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --always --dirty
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_VERSION
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # Use git describe output only when its embedded tag matches package.json version.
    # Exact tag → clean release header. Ahead-of-tag → local dev header with hash.
    # When package.json is bumped ahead of the last tag (release commit in progress),
    # the versions differ and we fall back to v${PROJECT_VERSION} so the tagged
    # commit on GitHub always gets a clean version string.
    if(GIT_VERSION MATCHES "^v([0-9]+\\.[0-9]+\\.[0-9]+)")
        set(GIT_TAG_VER "${CMAKE_MATCH_1}")
    else()
        set(GIT_TAG_VER "")
    endif()
    if(GIT_TAG_VER VERSION_EQUAL PROJECT_VERSION AND NOT GIT_VERSION MATCHES "dirty")
        set(RLOG_FULL_VERSION "${GIT_VERSION}")
    endif()

    # Git Hook Automation
    set(HOOK_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/scripts/pre-commit.sh")
    set(HOOK_DEST "${CMAKE_CURRENT_SOURCE_DIR}/.git/hooks/pre-commit")
    
    if(EXISTS "${HOOK_SOURCE}")
        if(UNIX)
            execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink "${HOOK_SOURCE}" "${HOOK_DEST}")
            file(CHMOD "${HOOK_SOURCE}" PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
        else()
            execute_process(COMMAND ${CMAKE_COMMAND} -E copy "${HOOK_SOURCE}" "${HOOK_DEST}")
        endif()
        message(STATUS "Git pre-commit hook installed/updated.")
    endif()

endif()

# Generate the header file directly in the source include directory
set(RLOG_SOURCE_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/include/rlog/rlog.hpp")
set(RLOG_BINARY_HEADER "${CMAKE_CURRENT_BINARY_DIR}/include/rlog/rlog.hpp")

# 1. Generate the header file in the source directory (the "master" for distribution)
configure_file(rlog.hpp.in "${RLOG_SOURCE_HEADER}" @ONLY)

# 2. Ensure the build include directory exists
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include/rlog")

# 3. Create a symbolic link in the build directory; fallback to copy on non-Unix systems
if(UNIX)
    file(CREATE_LINK "${RLOG_SOURCE_HEADER}" "${RLOG_BINARY_HEADER}" SYMBOLIC)
else()
    file(CREATE_LINK "${RLOG_SOURCE_HEADER}" "${RLOG_BINARY_HEADER}" COPY)
endif()

# Header-only library target
add_library(rlog INTERFACE)
add_library(rlog::rlog ALIAS rlog)

target_include_directories(rlog INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Dependencies
find_package(Intl REQUIRED)
target_link_libraries(rlog INTERFACE ${Intl_LIBRARIES})

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
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/rlog/rlog.hpp" DESTINATION include/rlog)
install(EXPORT rlogTargets
    FILE rlogConfig.cmake
    DESTINATION lib/cmake/rlog)
