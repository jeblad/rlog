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

# xgettext arguments derived from the macros defined in rlog.hpp.
# Consumers retrieve this via get_target_property(... rlog RLOG_XGETTEXT_ARGS)
# and pass it to their CONFIGURE_GETTEXT / xgettext call.
set_property(TARGET rlog PROPERTY RLOG_XGETTEXT_ARGS
    "--keyword=_"
    "--keyword=N_:1"
    "--keyword=n_:1,2"
    "--keyword=fmt_:1"
    "--keyword=nfmt_:1,2"
    "--keyword=EMERGENCY_:1" "--keyword=EMERGENCY_FMT_:1" "--keyword=EMERGENCY_N_:1,2" "--keyword=EMERGENCY_NFMT_:1,2"
    "--keyword=ALERT_:1"     "--keyword=ALERT_FMT_:1"     "--keyword=ALERT_N_:1,2"     "--keyword=ALERT_NFMT_:1,2"
    "--keyword=CRITICAL_:1"  "--keyword=CRITICAL_FMT_:1"  "--keyword=CRITICAL_N_:1,2"  "--keyword=CRITICAL_NFMT_:1,2"
    "--keyword=ERROR_:1"     "--keyword=ERROR_FMT_:1"     "--keyword=ERROR_N_:1,2"     "--keyword=ERROR_NFMT_:1,2"
    "--keyword=WARNING_:1"   "--keyword=WARNING_FMT_:1"   "--keyword=WARNING_N_:1,2"   "--keyword=WARNING_NFMT_:1,2"
    "--keyword=NOTICE_:1"    "--keyword=NOTICE_FMT_:1"    "--keyword=NOTICE_N_:1,2"    "--keyword=NOTICE_NFMT_:1,2"
    "--keyword=INFO_:1"      "--keyword=INFO_FMT_:1"      "--keyword=INFO_N_:1,2"      "--keyword=INFO_NFMT_:1,2"
    "--keyword=DEBUG_:1"     "--keyword=DEBUG_FMT_:1"     "--keyword=DEBUG_N_:1,2"     "--keyword=DEBUG_NFMT_:1,2"
    "--keyword=USER_:1"      "--keyword=USER_FMT_:1"
    "--flag=fmt_:1:csharp-format"
    "--flag=nfmt_:1:csharp-format"  "--flag=nfmt_:2:csharp-format"
    "--flag=USER_FMT_:1:csharp-format"
    "--flag=EMERGENCY_FMT_:1:csharp-format" "--flag=EMERGENCY_NFMT_:1:csharp-format" "--flag=EMERGENCY_NFMT_:2:csharp-format"
    "--flag=ALERT_FMT_:1:csharp-format"     "--flag=ALERT_NFMT_:1:csharp-format"     "--flag=ALERT_NFMT_:2:csharp-format"
    "--flag=CRITICAL_FMT_:1:csharp-format"  "--flag=CRITICAL_NFMT_:1:csharp-format"  "--flag=CRITICAL_NFMT_:2:csharp-format"
    "--flag=ERROR_FMT_:1:csharp-format"     "--flag=ERROR_NFMT_:1:csharp-format"     "--flag=ERROR_NFMT_:2:csharp-format"
    "--flag=WARNING_FMT_:1:csharp-format"   "--flag=WARNING_NFMT_:1:csharp-format"   "--flag=WARNING_NFMT_:2:csharp-format"
    "--flag=NOTICE_FMT_:1:csharp-format"    "--flag=NOTICE_NFMT_:1:csharp-format"    "--flag=NOTICE_NFMT_:2:csharp-format"
    "--flag=INFO_FMT_:1:csharp-format"      "--flag=INFO_NFMT_:1:csharp-format"      "--flag=INFO_NFMT_:2:csharp-format"
    "--flag=DEBUG_FMT_:1:csharp-format"     "--flag=DEBUG_NFMT_:1:csharp-format"     "--flag=DEBUG_NFMT_:2:csharp-format"
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
