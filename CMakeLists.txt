cmake_minimum_required(VERSION 3.11)

project(winutil VERSION 1.0.0 LANGUAGES CXX)

# Options
option(BUILD_TESTS "Build test executables" OFF)
option(BUILD_SHARED_LIBS "Build shared library" ON)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler options
add_compile_options(-O3)

# Include directories
set(WINUTIL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(WINUTIL_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/winutil")

# Collect source files
file(GLOB_RECURSE WINUTIL_SRC_FILES "${WINUTIL_SOURCE_DIR}/*.cpp")

# Create library
add_library(winutil ${WINUTIL_SRC_FILES})
target_include_directories(winutil
    PUBLIC
        $<BUILD_INTERFACE:${WINUTIL_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Set version properties
set_target_properties(winutil PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    EXPORT_NAME winutil
)

# Export compile commands
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Installation
include(GNUInstallDirs)

install(TARGETS winutil
    EXPORT winutilTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY ${WINUTIL_INCLUDE_DIR}/winutil
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp"
)

install(EXPORT winutilTargets
    FILE winutilTargets.cmake
    NAMESPACE winutil::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/winutil
)

# Export for FetchContent
export(TARGETS winutil FILE "${CMAKE_CURRENT_BINARY_DIR}/winutilTargets.cmake")

# Create config file for find_package
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/winutil-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/winutil-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/winutil
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/winutil-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/winutil-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/winutil-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/winutil
)

# Tests (conditional)
if(BUILD_TESTS)
    find_package(Catch2 3 REQUIRED)

    set(TEST_DIR "${PROJECT_SOURCE_DIR}/test")
    file(GLOB TEST_SRC_FILES "${TEST_DIR}/test_*.cpp")

    foreach(test_file ${TEST_SRC_FILES})
        get_filename_component(test_name ${test_file} NAME_WE)
        set(test_target "${test_name}")

        add_executable(${test_target} ${test_file})
        target_link_libraries(${test_target}
            PRIVATE
            winutil
            Catch2::Catch2WithMain
        )

        add_test(NAME ${test_target} COMMAND ${test_target})
    endforeach()

    add_executable(test_runner_all ${TEST_SRC_FILES})
    target_link_libraries(test_runner_all
        PRIVATE
        winutil
        Catch2::Catch2WithMain
    )

    add_test(NAME test_all COMMAND test_all)

    set_target_properties(test_runner_all PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test_bin"
    )

    enable_testing()
endif()
