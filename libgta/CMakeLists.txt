# Copyright (C) 2014
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

cmake_minimum_required(VERSION 2.8)
include(CheckTypeSize)

project(libgta C)

# Build options
option(GTA_BUILD_STATIC_LIB "Build static libgta" ON)
option(GTA_BUILD_SHARED_LIB "Build shared libgta" ON)
option(GTA_BUILD_DOCUMENTATION "Build API reference documentation (requires Doxygen)" ON)

# libgta version
set(GTA_VERSION_MAJOR "1")
set(GTA_VERSION_MINOR "0")
set(GTA_VERSION_PATCH "7")
set(GTA_VERSION 1.0.7)
set(GTA_VERSION_NUMBER 0x010007)
set(GTA_LIB_VERSION "0.0.4")
set(GTA_LIB_SOVERSION "0")

# Autoconf compatibility
set(PACKAGE_NAME "${PROJECT_NAME}")       # required by doxyfile.in
set(PACKAGE_VERSION "${GTA_VERSION}")     # required by doxyfile.in and gta.pc.in
set(PACKAGE_URL "http://gta.nongnu.org/") # required by gta.pc.in
set(top_srcdir ${CMAKE_SOURCE_DIR})       # required by doxyfile.in
check_type_size(int SIZEOF_INT)           # required by gta.c via config.h
check_type_size(int8_t SIZEOF_INT8_T)     # required by gta.c via config.h
file(WRITE "${CMAKE_BINARY_DIR}/src/config.h" "/* generated from CMakeLists.txt */\n")
file(APPEND "${CMAKE_BINARY_DIR}/src/config.h" "#define SIZEOF_INT ${SIZEOF_INT}\n")
file(APPEND "${CMAKE_BINARY_DIR}/src/config.h" "#define SIZEOF_INT8_T ${SIZEOF_INT8_T}\n")

# Compiler setup
if(CMAKE_COMPILER_IS_GNUCC)
  add_definitions(-std=gnu99)
endif()

# Main target: libgta
find_package(ZLIB REQUIRED)
find_package(BZip2 REQUIRED)
find_package(LibLZMA REQUIRED)
configure_file("${CMAKE_SOURCE_DIR}/src/gta/gta_version.h.in" "${CMAKE_BINARY_DIR}/src/gta/gta_version.h" @ONLY)
include_directories("${ZLIB_INCLUDE_DIRS}" "${BZIP2_INCLUDE_DIR}" "${LIBLZMA_INCLUDE_DIRS}"
  "${CMAKE_SOURCE_DIR}/src" "${CMAKE_BINARY_DIR}/src")
if(GTA_BUILD_SHARED_LIB)
  add_library(libgta_shared SHARED src/gta.c src/gta/gta.h src/gta/gta_version.h)
  target_link_libraries(libgta_shared ${ZLIB_LIBRARIES} ${BZIP2_LIBRARIES} ${LIBLZMA_LIBRARIES})
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
  set_target_properties(libgta_static PROPERTIES OUTPUT_NAME gta)
  install(TARGETS libgta_static
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION "lib${LIB_SUFFIX}"
    ARCHIVE DESTINATION "lib${LIB_SUFFIX}"
  )
endif()
install(FILES src/gta/gta.h "${CMAKE_BINARY_DIR}/src/gta/gta_version.h" DESTINATION include/gta)
# pkg-config file: gta.pc
set(prefix "${CMAKE_INSTALL_PREFIX}")
set(exec_prefix "\${prefix}")
set(libdir "\${exec_prefix}/lib${LIB_SUFFIX}")
set(includedir "\${prefix}/include")
set(GTA_PKGCONFIG_LIBRARIES_PRIVATE "")
foreach(GTA_PKGCONFIG_LIBRARY_PRIV ${LIBLZMA_LIBRARIES} ${BZIP2_LIBRARIES} ${ZLIB_LIBRARIES})
   set(GTA_PKGCONFIG_LIBRARIES_PRIVATE "${GTA_PKGCONFIG_LIBRARIES_PRIVATE} -l${GTA_PKGCONFIG_LIBRARY_PRIV}")
endforeach()
set(LTLIBLZMA ${GTA_PKGCONFIG_LIBRARIES_PRIVATE}) # for compatibility for libtool; see gta.pc.in
configure_file("${CMAKE_SOURCE_DIR}/src/gta.pc.in" "${CMAKE_BINARY_DIR}/src/gta.pc" @ONLY)
install(FILES "${CMAKE_BINARY_DIR}/src/gta.pc" DESTINATION lib${LIB_SUFFIX}/pkgconfig)

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
  install(FILES "${CMAKE_SOURCE_DIR}/cmake/FindGTA.cmake" DESTINATION share/libgta/cmake)
endif()

# Extra target: 'make dist' for making
set(ARCHIVE_NAME libgta-${GTA_VERSION})
add_custom_target(dist
  COMMAND git archive --prefix=${ARCHIVE_NAME}/ HEAD
    | xz > ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.xz
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
