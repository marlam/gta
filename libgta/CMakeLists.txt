# Copyright (C) 2014, 2015, 2016, 2018, 2019
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

cmake_minimum_required(VERSION 3.5)
include(CheckTypeSize)

project(libgta C)

# Build options
option(GTA_BUILD_STATIC_LIB "Build static libgta" ON)
option(GTA_BUILD_SHARED_LIB "Build shared libgta" ON)
option(GTA_BUILD_DOCUMENTATION "Build API reference documentation (requires Doxygen)" ON)

# libgta version
set(GTA_VERSION_MAJOR "1")
set(GTA_VERSION_MINOR "2")
set(GTA_VERSION_PATCH "0")
set(GTA_VERSION 1.2.0)
set(GTA_VERSION_NUMBER 0x010200)
set(GTA_LIB_VERSION "1.0.0")
set(GTA_LIB_SOVERSION "1")

# Autoconf compatibility
set(PACKAGE_NAME "${PROJECT_NAME}")       # required by doxyfile.in
set(PACKAGE_VERSION "${GTA_VERSION}")     # required by doxyfile.in and gta.pc.in
set(PACKAGE_URL "https://marlam.de/gta")  # required by gta.pc.in
set(top_srcdir ${CMAKE_SOURCE_DIR})       # required by doxyfile.in
check_type_size(int SIZEOF_INT)           # required by gta.c via config.h
check_type_size(int8_t SIZEOF_INT8_T)     # required by gta.c via config.h
file(WRITE "${CMAKE_BINARY_DIR}/src/config.h" "/* generated from CMakeLists.txt */\n")
file(APPEND "${CMAKE_BINARY_DIR}/src/config.h" "#define SIZEOF_INT ${SIZEOF_INT}\n")
file(APPEND "${CMAKE_BINARY_DIR}/src/config.h" "#define SIZEOF_INT8_T ${SIZEOF_INT8_T}\n")

# Main target: libgta
configure_file("${CMAKE_SOURCE_DIR}/src/gta/gta_version.h.in" "${CMAKE_BINARY_DIR}/src/gta/gta_version.h" @ONLY)
include_directories("${CMAKE_SOURCE_DIR}/src" "${CMAKE_BINARY_DIR}/src")
if(GTA_BUILD_SHARED_LIB)
  add_library(libgta_shared SHARED src/gta.c src/gta/gta.h src/gta/gta_version.h)
  set_property(TARGET libgta_shared PROPERTY C_STANDARD 99)
  set_target_properties(libgta_shared PROPERTIES DEFINE_SYMBOL DLL_EXPORT)
  set_target_properties(libgta_shared PROPERTIES OUTPUT_NAME gta)
  set_target_properties(libgta_shared PROPERTIES VERSION ${GTA_LIB_VERSION})
  set_target_properties(libgta_shared PROPERTIES SOVERSION ${GTA_LIB_SOVERSION})  
  install(TARGETS libgta_shared
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION "lib${LIB_SUFFIX}"
    ARCHIVE DESTINATION "lib${LIB_SUFFIX}"
  )
endif()
if(GTA_BUILD_STATIC_LIB)
  add_library(libgta_static STATIC src/gta.c src/gta/gta.h src/gta/gta_version.h)
  set_property(TARGET libgta_static PROPERTY C_STANDARD 99)
  set_target_properties(libgta_static PROPERTIES OUTPUT_NAME gta)
  install(TARGETS libgta_static
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION "lib${LIB_SUFFIX}"
    ARCHIVE DESTINATION "lib${LIB_SUFFIX}"
  )
endif()
install(FILES src/gta/gta.h src/gta/gta.hpp "${CMAKE_BINARY_DIR}/src/gta/gta_version.h" DESTINATION include/gta)
# pkg-config file: gta.pc
set(prefix "${CMAKE_INSTALL_PREFIX}")
set(exec_prefix "\${prefix}")
set(libdir "\${exec_prefix}/lib${LIB_SUFFIX}")
set(includedir "\${prefix}/include")
configure_file("${CMAKE_SOURCE_DIR}/src/gta.pc.in" "${CMAKE_BINARY_DIR}/src/gta.pc" @ONLY)
install(FILES "${CMAKE_BINARY_DIR}/src/gta.pc" DESTINATION lib${LIB_SUFFIX}/pkgconfig)
# CMake module files
include(CMakePackageConfigHelpers)
set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include)
set(LIB_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
configure_package_config_file(
    GTAConfig.cmake.in ${CMAKE_BINARY_DIR}/GTAConfig.cmake
    INSTALL_DESTINATION lib${LIB_SUFFIX}/cmake/GTA-{GTA_VERSION}
    PATH_VARS INCLUDE_INSTALL_DIR LIB_INSTALL_DIR
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)
write_basic_package_version_file(
    ${CMAKE_BINARY_DIR}/GTAConfigVersion.cmake
    VERSION ${GTA_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(FILES
    ${CMAKE_BINARY_DIR}/GTAConfig.cmake
    ${CMAKE_BINARY_DIR}/GTAConfigVersion.cmake
    DESTINATION ${LIB_INSTALL_DIR}/cmake/GTA-${GTA_VERSION}
)

# Optional target: reference documentation
if(GTA_BUILD_DOCUMENTATION)
  find_package(Doxygen REQUIRED)
  configure_file("${CMAKE_SOURCE_DIR}/doc/doxyfile.in" "${CMAKE_BINARY_DIR}/doc/doxyfile" @ONLY)
  file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/doc/reference")
  add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/doc/reference/index.html"
    COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_BINARY_DIR}/doc/doxyfile"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/doc"
    DEPENDS "${CMAKE_SOURCE_DIR}/doc/doxyfile.in" "${CMAKE_SOURCE_DIR}/src/gta/gta.h" "${CMAKE_SOURCE_DIR}/src/gta/gta.hpp"
    COMMENT "Generating API documentation with Doxygen" VERBATIM
  )
  add_custom_target(doc ALL DEPENDS "${CMAKE_BINARY_DIR}/doc/reference/index.html")
  install(DIRECTORY "${CMAKE_BINARY_DIR}/doc/reference" DESTINATION share/doc/libgta)
endif()

# Extra target: 'make dist' for making
set(ARCHIVE_NAME libgta-${GTA_VERSION})
add_custom_target(dist
  COMMAND git archive --prefix=${ARCHIVE_NAME}/ HEAD
    | xz > ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.xz
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
