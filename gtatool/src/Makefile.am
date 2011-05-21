# Copyright (C) 2010, 2011
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

SUBDIRS = base

BUILT_SOURCES =
CLEANFILES =
EXTRA_DIST =
AM_CPPFLAGS = -I$(top_srcdir)/src/base -I$(top_srcdir)/src -DPKGLIBDIR=\"$(pkglibdir)\" \
	$(libgta_CFLAGS) \
	$(libmuparser_CFLAGS) \
	$(libmagick_CFLAGS) \
	$(libpfs_CFLAGS) \
	$(libopenexr_CFLAGS) \
	$(libgdal_CFLAGS) \
	$(libmatio_CFLAGS) \
	$(libqtgui_CFLAGS)

#
# The gta program
#

bin_PROGRAMS = gta
gta_SOURCES = main.cpp cmds.h cmds.cpp lib.h lib.cpp
gta_LDADD = libbuiltin.la base/libbase.la $(libgta_LIBS) $(LIB_DLOPEN) $(LIBICONV)
gta_LDFLAGS = -export-dynamic
if W32
gta_SOURCES += appicon.ico
.ico.o:
	echo "IDI_ICON1 ICON DISCARDABLE \"$<\"" | $(WINDRES) -o $@
endif

#
# Builtin commands
#

noinst_LTLIBRARIES = libbuiltin.la
libbuiltin_la_SOURCES = \
	component/component-add.cpp \
	component/component-convert.cpp \
	component/component-extract.cpp \
	component/component-merge.cpp \
	component/component-reorder.cpp \
	component/component-set.cpp \
	component/component-split.cpp \
	dimension/dimension-add.cpp \
	dimension/dimension-extract.cpp \
	dimension/dimension-merge.cpp \
	dimension/dimension-reorder.cpp \
	dimension/dimension-reverse.cpp \
	dimension/dimension-split.cpp \
	array/compress.cpp \
	array/create.cpp \
	array/extract.cpp \
	array/fill.cpp \
	array/info.cpp \
	array/merge.cpp \
	array/resize.cpp \
	array/set.cpp \
	array/tag.cpp \
	array/uncompress.cpp \
	stream/stream-extract.cpp \
	stream/stream-merge.cpp \
	stream/stream-split.cpp \
	format-raw/from-raw.cpp \
	format-raw/to-raw.cpp \
        format-rat/from-rat.cpp \
        format-rat/to-rat.cpp
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
component_compute_la_SOURCES = component/component-compute.cpp
component_compute_la_LIBADD = $(libmuparser_LIBS)
else
libbuiltin_la_SOURCES += component/component-compute.cpp
libbuiltin_la_LIBADD += $(libmuparser_LIBS)
endif
endif

if WITH_NETPBM
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += format-netpbm.la
format_netpbm_la_SOURCES = format-netpbm/from-netpbm.cpp format-netpbm/to-netpbm.cpp
format_netpbm_la_LIBADD = $(LTLIBNETPBM)
else
libbuiltin_la_SOURCES += format-netpbm/from-netpbm.cpp format-netpbm/to-netpbm.cpp
libbuiltin_la_LIBADD += $(LTLIBNETPBM)
endif
endif

if WITH_MAGICK
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += format-magick.la
format_magick_la_SOURCES = format-magick/from-magick.cpp format-magick/to-magick.cpp
format_magick_la_LIBADD = $(libmagick_LIBS)
else
libbuiltin_la_SOURCES += format-magick/from-magick.cpp format-magick/to-magick.cpp
libbuiltin_la_LIBADD += $(libmagick_LIBS)
endif
endif

if WITH_PFS
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += format-pfs.la
format_pfs_la_SOURCES = format-pfs/from-pfs.cpp format-pfs/to-pfs.cpp
format_pfs_la_LIBADD = $(libpfs_LIBS)
else
libbuiltin_la_SOURCES += format-pfs/from-pfs.cpp format-pfs/to-pfs.cpp
libbuiltin_la_LIBADD += $(libpfs_LIBS)
endif
endif

if WITH_EXR
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += format-exr.la
format_exr_la_SOURCES = format-exr/from-exr.cpp format-exr/to-exr.cpp
format_exr_la_LIBADD = $(libopenexr_LIBS)
else
libbuiltin_la_SOURCES += format-exr/from-exr.cpp format-exr/to-exr.cpp
libbuiltin_la_LIBADD += $(libopenexr_LIBS)
endif
endif

if WITH_GDAL
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += format-gdal.la
format_gdal_la_SOURCES = format-gdal/from-gdal.cpp format-gdal/to-gdal.cpp
format_gdal_la_LIBADD = $(libgdal_LIBS)
else
libbuiltin_la_SOURCES += format-gdal/from-gdal.cpp format-gdal/to-gdal.cpp
libbuiltin_la_LIBADD += $(libgdal_LIBS)
endif
endif

if WITH_DCMTK
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += format-dcmtk.la
format_dcmtk_la_SOURCES = format-dcmtk/from-dcmtk.cpp
format_dcmtk_la_LIBADD = $(LTLIBDCMIMGLE)
else
libbuiltin_la_SOURCES += format-dcmtk/from-dcmtk.cpp
libbuiltin_la_LIBADD += $(LTLIBDCMIMGLE)
endif
endif

if WITH_MAT
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += format-mat.la
format_mat_la_SOURCES = format-mat/from-mat.cpp format-mat/to-mat.cpp
format_mat_la_LIBADD = $(libmatio_LIBS)
else
libbuiltin_la_SOURCES += format-mat/from-mat.cpp format-mat/to-mat.cpp
libbuiltin_la_LIBADD += $(libmatio_LIBS)
endif
endif

if WITH_QT
moc_verbose = $(moc_verbose_$(V))
moc_verbose_ = $(moc_verbose_$(AM_DEFAULT_VERBOSITY))
moc_verbose_0 = @echo "  MOC   " $@;
%.moc.cpp: %.h
	@$(MKDIR_P) gui
	$(moc_verbose)$(MOC) -o $@ $<
rcc_verbose = $(rcc_verbose_$(V))
rcc_verbose_ = $(rcc_verbose_$(AM_DEFAULT_VERBOSITY))
rcc_verbose_0 = @echo "  RCC   " $@;
%.resources.cpp: %.qrc
	@$(MKDIR_P) gui
	$(rcc_verbose)$(RCC) -o $@ $<
GUI_BUILT_SOURCES = gui/gui.moc.cpp gui/gui.resources.cpp
BUILT_SOURCES += $(GUI_BUILT_SOURCES)
CLEANFILES += $(GUI_BUILT_SOURCES)
EXTRA_DIST += gui/gui.png gui/gui.qrc
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += gui.la
gui_la_SOURCES = gui/gui.h gui/gui.cpp
nodist_gui_la_SOURCES = $(GUI_BUILT_SOURCES)
gui_la_LIBADD = $(libqtgui_LIBS)
else
libbuiltin_la_SOURCES += gui/gui.h gui/gui.cpp
nodist_libbuiltin_la_SOURCES += $(GUI_BUILT_SOURCES)
libbuiltin_la_LIBADD += $(libqtgui_LIBS)
endif
endif
