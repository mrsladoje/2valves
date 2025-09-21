cmake_minimum_required(VERSION 3.21)

# Set vcpkg toolchain before project()
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "")
endif()

project(2valves VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
set(JSON_BuildTests OFF CACHE INTERNAL "")
find_package(cpr CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(TBB CONFIG REQUIRED)

# Find lexbor - prioritize vcpkg on Windows
set(LEXBOR_FOUND FALSE)

# Method 1: Try vcpkg/CMake config first (preferred for vcpkg)
find_package(lexbor CONFIG QUIET)
if(lexbor_FOUND OR TARGET lexbor::lexbor)
    set(LEXBOR_LIBRARIES lexbor::lexbor)
    set(LEXBOR_FOUND TRUE)
    message(STATUS "Found lexbor via CMake config (vcpkg)")
endif()

# Method 2: Try pkg-config (only on Unix-like systems)
if(NOT LEXBOR_FOUND AND UNIX)
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(LEXBOR QUIET lexbor)
        if(LEXBOR_FOUND)
            message(STATUS "Found lexbor via pkg-config")
        endif()
    endif()
endif()

# Method 3: Manual search as last resort
if(NOT LEXBOR_FOUND)
    message(STATUS "Falling back to manual vcpkg lexbor setup...")
    set(LEXBOR_INCLUDE_DIRS "$ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/include")
    set(LEXBOR_LIBRARIES "$ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/lib/lexbor.lib")
    if(EXISTS "${LEXBOR_LIBRARIES}")
        set(LEXBOR_FOUND TRUE)
        message(STATUS "Found lexbor manually in vcpkg")
        message(STATUS "  Include: ${LEXBOR_INCLUDE_DIRS}")
        message(STATUS "  Library: ${LEXBOR_LIBRARIES}")
    endif()
endif()

# Collect all source files
file(GLOB_RECURSE SOURCES "*.cpp" "*.h")
list(FILTER SOURCES EXCLUDE REGEX ".*/build/.*")

# Create executable
add_executable(article_scraper ${SOURCES})

# Link libraries
target_link_libraries(article_scraper 
    PRIVATE 
    cpr::cpr
    nlohmann_json::nlohmann_json
    TBB::tbb
    ${LEXBOR_LIBRARIES}
)

# Include directories
target_include_directories(article_scraper 
    PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${LEXBOR_INCLUDE_DIRS}
)

# Compiler flags for lexbor (if available from pkg-config)
if(LEXBOR_CFLAGS_OTHER)
    target_compile_options(article_scraper PRIVATE ${LEXBOR_CFLAGS_OTHER})
endif()

# Platform-specific configurations
if(WIN32)
    # Windows-specific settings
    if(MSVC)
        target_compile_options(article_scraper PRIVATE /W4)
        # Disable specific MSVC warnings if needed
        # target_compile_definitions(article_scraper PRIVATE _CRT_SECURE_NO_WARNINGS)
    endif()
elseif(UNIX AND NOT APPLE)
    # Linux-specific settings
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(article_scraper PRIVATE -Wall -Wextra)
    endif()
elseif(APPLE)
    # macOS-specific settings
    target_compile_options(article_scraper PRIVATE -Wall -Wextra)
endif()

# Debug information
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Lexbor found: ${LEXBOR_FOUND}")
if(LEXBOR_FOUND)
    message(STATUS "Lexbor libraries: ${LEXBOR_LIBRARIES}")
    if(LEXBOR_INCLUDE_DIRS)
        message(STATUS "Lexbor include dirs: ${LEXBOR_INCLUDE_DIRS}")
    endif()
endif()