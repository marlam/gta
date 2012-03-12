# - Find the GTA includes and library
#
# This module accepts the following environment variables:
#  GTA_ROOT - Specify the location of libgta
#
# This module defines
#  GTA_FOUND - If false, do not try to use libgta.
#  GTA_INCLUDE_DIRS - Where to find the headers.
#  GTA_LIBRARIES - The libraries to link against to use libgta.

# Copyright (C) 2012
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

INCLUDE(FindPkgConfig OPTIONAL)

IF(PKG_CONFIG_FOUND)
    INCLUDE(FindPkgConfig)
    PKG_CHECK_MODULES(GTA gta)
ELSE(PKG_CONFIG_FOUND)
    FIND_PATH(GTA_INCLUDE_DIRS gta/gta.h
        $ENV{GTA_ROOT}/include
        $ENV{GTA_ROOT}
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/include
        /usr/include
        /sw/include # Fink
        /opt/local/include # DarwinPorts
        /opt/csw/include # Blastwave
        /opt/include
        /usr/freeware/include
    )
    FIND_LIBRARY(GTA_LIBRARIES 
        NAMES gta libgta
        PATHS
        $ENV{GTA_ROOT}/lib
        $ENV{GTA_ROOT}
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/lib
        /usr/lib
        /sw/lib
        /opt/local/lib
        /opt/csw/lib
        /opt/lib
        /usr/freeware/lib64
    )
    SET(GTA_FOUND "NO")
    IF(GTA_LIBRARIES AND GTA_INCLUDE_DIRS)
        SET(GTA_FOUND "YES")
    ENDIF(GTA_LIBRARIES AND GTA_INCLUDE_DIRS)
ENDIF(PKG_CONFIG_FOUND)
