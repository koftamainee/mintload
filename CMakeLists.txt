cmake_minimum_required(VERSION 3.15)
project(mintload VERSION 1.0.0 LANGUAGES C)

option(BUILD_SHARED_LIBS "Build mintload as shared library" OFF)
option(MINTLOAD_BUILD_TESTS "Build mintload tests" ON)

set(HEADERS
    include/mintload/mintload.h
    include/mintload/mmesh.h
    include/mintload/mmat.h
)

set(SOURCES
    src/mmesh.c
    src/mmat.c
)

add_library(mintload ${SOURCES})

target_include_directories(mintload
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set_target_properties(mintload PROPERTIES
    PUBLIC_HEADER "${HEADERS}"
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
)

if(NOT BUILD_SHARED_LIBS)
    set_target_properties(mintload PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

if(WIN32 AND BUILD_SHARED_LIBS)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

include(GNUInstallDirs)

install(TARGETS mintload
    EXPORT mintloadTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mintload
)

install(EXPORT mintloadTargets
    FILE mintloadTargets.cmake
    NAMESPACE mintload::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mintload
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/mintloadConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/mintloadConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mintload
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/mintloadConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/mintloadConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/mintloadConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mintload
)

export(TARGETS mintload FILE ${CMAKE_CURRENT_BINARY_DIR}/mintloadTargets.cmake NAMESPACE mintload::)

if(MINTLOAD_BUILD_TESTS)
    enable_testing()
    add_executable(test_mintload
        tests/test_mmesh.c
        tests/test_mmat.c
        tests/test_utils.c
        tests/test_mintload.c
    )
    target_link_libraries(test_mintload mintload)
    target_include_directories(test_mintload PRIVATE include)
    add_test(NAME mintload_test COMMAND test_mintload)
endif()
