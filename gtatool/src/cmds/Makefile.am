# Copyright (C) 2010  Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

AM_CPPFLAGS = -I$(top_srcdir)/src/base -I$(top_srcdir)/src
BUILT_SOURCES =
CLEANFILES =
EXTRA_DIST =

#
# Builtin commands
#

noinst_LTLIBRARIES = libbuiltin.la
libbuiltin_la_SOURCES = \
	component-add.cpp	\
	component-convert.cpp	\
	component-extract.cpp	\
	component-merge.cpp	\
	component-reorder.cpp	\
	component-set.cpp	\
	component-split.cpp	\
	dimension-add.cpp	\
	dimension-extract.cpp	\
	dimension-merge.cpp	\
	dimension-reorder.cpp	\
	dimension-reverse.cpp	\
	dimension-split.cpp	\
	compress.cpp		\
	create.cpp		\
	extract.cpp		\
	fill.cpp		\
	info.cpp		\
	merge.cpp		\
	resize.cpp		\
	set.cpp			\
	tag.cpp			\
	uncompress.cpp		\
	stream-extract.cpp	\
	stream-merge.cpp	\
	stream-split.cpp	\
	from-raw.cpp		\
	to-raw.cpp
nodist_libbuiltin_la_SOURCES =
libbuiltin_la_LIBADD =

#
# Dynamic commands
#

if DYNAMIC_MODULES
pkglib_LTLIBRARIES =
AM_LDFLAGS = -module -avoid-version -shrext .so
# Remove unused module.la files from pkglibdir
# FIXME: Is there a way to tell automake/libtool that we do not want these files installed?
install-exec-hook:
	LA_FILES="$(foreach la_file,$(pkglib_LTLIBRARIES),$(pkglibdir)/$(la_file))"; test -z "$$LA_FILES" || rm $$LA_FILES
else
# Remove the empty pkglibdir and, if it is otherwise empty, its parent directory
# FIXME: Is there a way to tell automake that pkglibdir needs not be created?
install-exec-hook:
	rmdir $(pkglibdir)
	rmdir $(libdir) || true
endif

if WITH_MUPARSER
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += component-compute.la
component_compute_la_SOURCES = component-compute.cpp
component_compute_la_LIBADD = $(LTLIBMUPARSER)
else
libbuiltin_la_SOURCES += component-compute.cpp
libbuiltin_la_LIBADD += $(LTLIBMUPARSER)
endif
endif

if WITH_NETPBM
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += from-netpbm.la
from_netpbm_la_SOURCES = from-netpbm.cpp
from_netpbm_la_LIBADD = $(LTLIBNETPBM)
pkglib_LTLIBRARIES += to-netpbm.la
to_netpbm_la_SOURCES = to-netpbm.cpp
to_netpbm_la_LIBADD = $(LTLIBNETPBM)
else
libbuiltin_la_SOURCES += from-netpbm.cpp to-netpbm.cpp
libbuiltin_la_LIBADD += $(LTLIBNETPBM)
endif
endif

if WITH_MAGICK
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += from-magick.la
from_magick_la_SOURCES = from-magick.cpp
from_magick_la_LIBADD = $(LTLIBMAGICK)
pkglib_LTLIBRARIES += to-magick.la
to_magick_la_SOURCES = to-magick.cpp
to_magick_la_LIBADD = $(LTLIBMAGICK)
else
libbuiltin_la_SOURCES += from-magick.cpp to-magick.cpp
libbuiltin_la_LIBADD += $(LTLIBMAGICK)
endif
endif

if WITH_PFS
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += from-pfs.la
from_pfs_la_SOURCES = from-pfs.cpp
from_pfs_la_LIBADD = $(LTLIBPFS_1_2)
pkglib_LTLIBRARIES += to-pfs.la
to_pfs_la_SOURCES = to-pfs.cpp
to_pfs_la_LIBADD = $(LTLIBPFS_1_2)
else
libbuiltin_la_SOURCES += from-pfs.cpp to-pfs.cpp
libbuiltin_la_LIBADD += $(LTLIBPFS_1_2)
endif
endif

if WITH_EXR
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += from-exr.la
from_exr_la_SOURCES = from-exr.cpp
from_exr_la_LIBADD = $(LTLIBILMIMF)
pkglib_LTLIBRARIES += to-exr.la
to_exr_la_SOURCES = to-exr.cpp
to_exr_la_LIBADD = $(LTLIBILMIMF)
else
libbuiltin_la_SOURCES += from-exr.cpp to-exr.cpp
libbuiltin_la_LIBADD += $(LTLIBILMIMF)
endif
endif

if WITH_GDAL
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += from-gdal.la
from_gdal_la_SOURCES = from-gdal.cpp
from_gdal_la_LIBADD = $(LTLIBGDAL)
pkglib_LTLIBRARIES += to-gdal.la
to_gdal_la_SOURCES = to-gdal.cpp
to_gdal_la_LIBADD = $(LTLIBGDAL)
else
libbuiltin_la_SOURCES += from-gdal.cpp to-gdal.cpp
libbuiltin_la_LIBADD += $(LTLIBGDAL)
endif
endif

if WITH_DCMTK
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += from-dcmtk.la
from_dcmtk_la_SOURCES = from-dcmtk.cpp
from_dcmtk_la_LIBADD = $(LTLIBDCMIMGLE)
else
libbuiltin_la_SOURCES += from-dcmtk.cpp
libbuiltin_la_LIBADD += $(LTLIBDCMIMGLE)
endif
endif

if WITH_QT
moc_verbose = $(moc_verbose_$(V))
moc_verbose_ = $(moc_verbose_$(AM_DEFAULT_VERBOSITY))
moc_verbose_0 = @echo "  MOC   " $@;
%.moc.cpp: %.h
	$(moc_verbose)$(MOC) -o $@ $<
rcc_verbose = $(rcc_verbose_$(V))
rcc_verbose_ = $(rcc_verbose_$(AM_DEFAULT_VERBOSITY))
rcc_verbose_0 = @echo "  RCC   " $@;
%.resources.cpp: %.qrc
	$(rcc_verbose)$(RCC) -o $@ $<
GUI_BUILT_SOURCES = gui.moc.cpp gui.resources.cpp
BUILT_SOURCES += $(GUI_BUILT_SOURCES)
CLEANFILES += $(GUI_BUILT_SOURCES)
EXTRA_DIST += gui.png gui.qrc
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += gui.la
gui_la_SOURCES = gui.h gui.cpp
nodist_gui_la_SOURCES = $(GUI_BUILT_SOURCES)
gui_la_LIBADD = $(LTLIBQTGUI)
else
libbuiltin_la_SOURCES += gui.h gui.cpp
nodist_libbuiltin_la_SOURCES += $(GUI_BUILT_SOURCES)
libbuiltin_la_LIBADD += $(LTLIBQTGUI)
endif
endif
