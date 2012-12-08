# - Try to find the GTA library (libgta)
#
# Once done this will define
#
#  GTA_FOUND - System has libgta
#  GTA_INCLUDE_DIR - The libgta include directory
#  GTA_LIBRARIES - The libraries needed to use libgta

# Adapted from FindGnuTLS.cmake 2012-12-06, Martin Lambers.
# Original copyright notice:
#=============================================================================
# Copyright 2009 Kitware, Inc.
# Copyright 2009 Philip Lowman <philip@yhbt.com>
# Copyright 2009 Brad Hards <bradh@kde.org>
# Copyright 2006 Alexander Neundorf <neundorf@kde.org>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)


IF(GTA_INCLUDE_DIR AND GTA_LIBRARY)
    # in cache already
    SET(GTA_FIND_QUIETLY TRUE)
ENDIF(GTA_INCLUDE_DIR AND GTA_LIBRARY)

FIND_PACKAGE(PkgConfig QUIET)
IF(PKG_CONFIG_FOUND)
    # try using pkg-config to get the directories and then use these values
    # in the FIND_PATH() and FIND_LIBRARY() calls
    PKG_CHECK_MODULES(PC_GTA QUIET gta)
    SET(GTA_VERSION_STRING ${PC_GTA_VERSION})
ENDIF()

FIND_PATH(GTA_INCLUDE_DIR gta/gta.h HINTS ${PC_GTA_INCLUDE_DIRS})

FIND_LIBRARY(GTA_LIBRARY NAMES gta libgta HINTS ${PC_GTA_LIBRARY_DIRS})

MARK_AS_ADVANCED(GTA_INCLUDE_DIR GTA_LIBRARY)

# handle the QUIETLY and REQUIRED arguments and set GTA_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GTA
    REQUIRED_VARS GTA_LIBRARY GTA_INCLUDE_DIR
    VERSION_VAR GTA_VERSION_STRING
)

IF(GTA_FOUND)
    SET(GTA_LIBRARIES ${GTA_LIBRARY})
    SET(GTA_INCLUDE_DIRS ${GTA_INCLUDE_DIR})
ENDIF()
