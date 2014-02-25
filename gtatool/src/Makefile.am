# Copyright (C) 2010, 2011, 2012, 2013, 2014
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

SUBDIRS = base

BUILT_SOURCES =
CLEANFILES =
EXTRA_DIST =
AM_CPPFLAGS = -I$(top_srcdir)/src -DPKGLIBDIR=\"$(pkglibdir)\" \
	$(libgta_CFLAGS) \
	$(libmuparser_CFLAGS) \
	$(libqtgui_CFLAGS) \
	$(libopenexr_CFLAGS) \
	$(libffmpeg_CFLAGS) \
	$(libgdal_CFLAGS) \
	$(libmagick_CFLAGS) \
	$(libmatio_CFLAGS) \
	$(libnetcdf_CFLAGS) \
	$(libpcl_io_CFLAGS) \
	$(libpfs_CFLAGS) \
	$(libsndfile_CFLAGS) \
	$(libteem_CFLAGS) \
	$(libqtopengl_CFLAGS) \
	$(libglewmx_CFLAGS) \
	$(libequalizer_CFLAGS) \
	$(libgls_CFLAGS) \
	$(libgl_CFLAGS)

#
# The gta program
#

bin_PROGRAMS = gta
gta_SOURCES = main.cpp cmds.h cmds.cpp lib.h lib.cpp
gta_LDADD = libbuiltin.la base/libbase.la $(libgta_LIBS) $(LIB_DLOPEN) $(LIBICONV)
if DYNAMIC_MODULES
gta_LDFLAGS = -export-dynamic
endif
if W32
gta_SOURCES += appicon.ico
.ico.o:
	echo "IDI_ICON1 ICON DISCARDABLE \"$<\"" | $(WINDRES) -o $@
endif
if WITH_BASHCOMPLETION
bashcompletiondir = $(BASHCOMPLETIONDIR)
dist_bashcompletion_DATA = bash-completion/gta
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
	dimension/dimension-flatten.cpp \
	dimension/dimension-merge.cpp \
	dimension/dimension-reorder.cpp \
	dimension/dimension-reverse.cpp \
	dimension/dimension-split.cpp \
	array/combine.cpp \
	array/compress.cpp \
	array/create.cpp \
	array/diff.cpp \
	array/extract.cpp \
	array/fill.cpp \
	array/info.cpp \
	array/merge.cpp \
	array/resize.cpp \
	array/set.cpp \
	array/tag.cpp \
	array/uncompress.cpp \
	stream/stream-extract.cpp \
	stream/stream-foreach.cpp \
	stream/stream-grep.cpp \
	stream/stream-merge.cpp \
	stream/stream-split.cpp \
	conv/from.cpp conv/to.cpp conv/filters.h conv/filters.cpp conv/conv.h conv/conv.cpp
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
	LA_FILES="$(foreach la_file,$(pkglib_LTLIBRARIES),$(DESTDIR)$(pkglibdir)/$(la_file))"; test -z "$$LA_FILES" || rm $$LA_FILES
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

if WITH_QT
moc_verbose = $(moc_verbose_$(V))
moc_verbose_ = $(moc_verbose_$(AM_DEFAULT_VERBOSITY))
moc_verbose_0 = @echo "  MOC     " $@;
%.moc.cpp: %.hpp
	@$(MKDIR_P) gui view
	$(moc_verbose)$(MOC) -o $@ $<
rcc_verbose = $(rcc_verbose_$(V))
rcc_verbose_ = $(rcc_verbose_$(AM_DEFAULT_VERBOSITY))
rcc_verbose_0 = @echo "  RCC     " $@;
%.resources.cpp: %.qrc
	@$(MKDIR_P) gui view
	$(rcc_verbose)$(RCC) -o $@ -name $(basename $(notdir $<)) $<
GUI_BUILT_SOURCES = gui/gui.moc.cpp gui/gui.resources.cpp
BUILT_SOURCES += $(GUI_BUILT_SOURCES)
CLEANFILES += $(GUI_BUILT_SOURCES)
EXTRA_DIST += gui/gui.qrc
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += gui.la
gui_la_SOURCES = gui/gui.hpp gui/gui.cpp gui/viewwidget.hpp
nodist_gui_la_SOURCES = $(GUI_BUILT_SOURCES)
gui_la_LIBADD = $(libqtgui_LIBS)
else
libbuiltin_la_SOURCES += gui/gui.hpp gui/gui.cpp gui/viewwidget.hpp
nodist_libbuiltin_la_SOURCES += $(GUI_BUILT_SOURCES)
libbuiltin_la_LIBADD += $(libqtgui_LIBS)
endif
iconsdatadir = $(datadir)/icons/hicolor
icons16dir = $(iconsdatadir)/16x16/apps
icons32dir = $(iconsdatadir)/32x32/apps
icons48dir = $(iconsdatadir)/48x48/apps
icons64dir = $(iconsdatadir)/64x64/apps
iconsscaldir = $(iconsdatadir)/scalable/apps
dist_icons16_DATA = gui/logo/16x16/gta.png
dist_icons32_DATA = gui/logo/32x32/gta.png
dist_icons48_DATA = gui/logo/48x48/gta.png
dist_icons64_DATA = gui/logo/64x64/gta.png
dist_iconsscal_DATA = gui/logo/scalable/gta.svg
update-icon-cache:
	test -n "$(DESTDIR)" -o -z "$(GTK_UPDATE_ICON_CACHE)" || (umask 022 && $(GTK_UPDATE_ICON_CACHE) -q -f -t "$(iconsdatadir)")
appdatadir = $(datadir)/applications
dist_appdata_DATA = gui/gta_gui.desktop
update-desktop-database:
	test -n "$(DESTDIR)" -o -z "$(UPDATE_DESKTOP_DATABASE)" || (umask 022 && $(UPDATE_DESKTOP_DATABASE) "$(appdatadir)")
install-data-hook: update-icon-cache update-desktop-database
uninstall-hook: update-icon-cache update-desktop-database
endif

if WITH_QTOPENGL
if WITH_GLEWMX
%.glsl.h: %.glsl
	$(AM_V_GEN)MACRONAME="`echo $< | sed -e s/^.*\\\/// -e s/\\\.glsl$$// -e s/[\\\.-]/_/g | tr [a-z] [A-Z]`_GLSL_STR"; \
	(echo "/* GENERATED AUTOMATICALLY FROM $< */"; \
	 echo "#ifndef $$MACRONAME"; \
	 echo "#define $$MACRONAME \\"; \
	 sed -e s/\\\\/\\\\\\\\/g \
	     -e s/\"/\\\\\"/g \
	     -e s/^/\"/ \
	     -e s/$$/\\\\n\"\\\\/ < $<; \
	 echo \"\"; \
	 echo "#endif") > $@
AM_CPPFLAGS += -I$(top_srcdir)/src/view -I$(top_builddir)/src/view
VIEW_SOURCES = \
	view/view.cpp view/widget.hpp view/widget.cpp \
	view/minmaxhist.hpp view/minmaxhist.cpp \
	view/mode_2d_widget.hpp view/mode_2d_widget.cpp \
	view/viewparams.hpp view/viewparams.cpp \
	view/renderer.hpp view/renderer.cpp \
	view/glwidget.hpp view/glwidget.cpp \
	view/xqglwidget.hpp view/xqglwidget.cpp \
	view/xgl/glmanager.hpp view/xgl/glmanager.cpp \
	view/xgl/glcontext.hpp view/xgl/glrenderer.hpp view/xgl/glwindow.hpp view/xgl/glnavigator.hpp \
	view/xgl/glvm.hpp view/xgl/glvm-ser.hpp \
	view/xgl/navigator.hpp view/xgl/navigator.cpp \
	view/base/ser.h view/base/ser.cpp \
	view/base/tmr.h view/base/tmr.cpp \
	view/base/sys.h view/base/sys.cpp
if WITH_EQUALIZER
VIEW_SOURCES += \
	view/eq/eqwindow.hpp view/eq/eqwindow.cpp \
	view/eq/tracking-eqevent.hpp view/eq/tracking-eqevent.cpp \
	view/eq/tracking.hpp view/eq/tracking.cpp
endif
VIEW_BUILT_SOURCES = gui/viewwidget.moc.cpp view/widget.moc.cpp \
	view/mode_2d_widget.moc.cpp view/glwidget.moc.cpp view/xqglwidget.moc.cpp \
	view/view.resources.cpp \
	view/mode_2d_fs.glsl.h
EXTRA_DIST += view/view.qrc \
	view/mode_2d_fs.glsl
BUILT_SOURCES += $(VIEW_BUILT_SOURCES)
CLEANFILES += $(VIEW_BUILT_SOURCES)
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += view.la
view_la_SOURCES = $(VIEW_SOURCES)
nodist_view_la_SOURCES = $(VIEW_BUILT_SOURCES)
view_la_LIBADD = $(libqtopengl_LIBS) $(libqtgui_LIBS) $(libequalizer_LIBS) $(libgls_LIBS) $(libglewmx_LIBS) $(libgl_LIBS)
else
libbuiltin_la_SOURCES += $(VIEW_SOURCES)
nodist_libbuiltin_la_SOURCES += $(VIEW_BUILT_SOURCES)
libbuiltin_la_LIBADD += $(libqtopengl_LIBS) $(libqtgui_LIBS) $(libequalizer_LIBS) $(libgls_LIBS) $(libglewmx_LIBS) $(libgl_LIBS)
endif
endif
endif

if WITH_CSV
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += conv-csv.la
conv_csv_la_SOURCES = conv-csv/delimiter.h conv-csv/delimiter.cpp conv-csv/from-csv.cpp conv-csv/to-csv.cpp
else
libbuiltin_la_SOURCES += conv-csv/delimiter.h conv-csv/delimiter.cpp conv-csv/from-csv.cpp conv-csv/to-csv.cpp
endif
endif

if WITH_DATRAW
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += conv-datraw.la
conv_datraw_la_SOURCES = conv-datraw/from-datraw.cpp conv-datraw/to-datraw.cpp
else
libbuiltin_la_SOURCES += conv-datraw/from-datraw.cpp conv-datraw/to-datraw.cpp
endif
endif

if WITH_DCMTK
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += conv-dcmtk.la
conv_dcmtk_la_SOURCES = conv-dcmtk/from-dcmtk.cpp
conv_dcmtk_la_LIBADD = $(LTLIBDCMIMAGE)
else
libbuiltin_la_SOURCES += conv-dcmtk/from-dcmtk.cpp
libbuiltin_la_LIBADD += $(LTLIBDCMIMAGE)
endif
endif

if WITH_EXR
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += conv-exr.la
conv_exr_la_SOURCES = conv-exr/from-exr.cpp conv-exr/to-exr.cpp
conv_exr_la_LIBADD = $(libopenexr_LIBS)
else
libbuiltin_la_SOURCES += conv-exr/from-exr.cpp conv-exr/to-exr.cpp
libbuiltin_la_LIBADD += $(libopenexr_LIBS)
endif
endif

if WITH_FFMPEG
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += conv-ffmpeg.la
conv_ffmpeg_la_SOURCES = conv-ffmpeg/from-ffmpeg.cpp \
	conv-ffmpeg/s11n.h conv-ffmpeg/s11n.cpp \
	conv-ffmpeg/thread.h conv-ffmpeg/thread.cpp \
	conv-ffmpeg/media_data.h conv-ffmpeg/media_data.cpp \
	conv-ffmpeg/media_object.h conv-ffmpeg/media_object.cpp
conv_ffmpeg_la_LIBADD = $(libffmpeg_LIBS)
else
libbuiltin_la_SOURCES += conv-ffmpeg/from-ffmpeg.cpp \
	conv-ffmpeg/s11n.h conv-ffmpeg/s11n.cpp \
	conv-ffmpeg/thread.h conv-ffmpeg/thread.cpp \
	conv-ffmpeg/media_data.h conv-ffmpeg/media_data.cpp \
	conv-ffmpeg/media_object.h conv-ffmpeg/media_object.cpp
libbuiltin_la_LIBADD += $(libffmpeg_LIBS)
endif
endif

if WITH_GDAL
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += conv-gdal.la
conv_gdal_la_SOURCES = conv-gdal/from-gdal.cpp conv-gdal/to-gdal.cpp
conv_gdal_la_LIBADD = $(libgdal_LIBS)
else
libbuiltin_la_SOURCES += conv-gdal/from-gdal.cpp conv-gdal/to-gdal.cpp
libbuiltin_la_LIBADD += $(libgdal_LIBS)
endif
endif

if WITH_JPEG
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += conv-jpeg.la
conv_jpeg_la_SOURCES = conv-jpeg/from-jpeg.cpp conv-jpeg/to-jpeg.cpp
conv_jpeg_la_LIBADD = $(LTLIBJPEG)
else
libbuiltin_la_SOURCES += conv-jpeg/from-jpeg.cpp conv-jpeg/to-jpeg.cpp
libbuiltin_la_LIBADD += $(LTLIBJPEG)
endif
endif

if WITH_MAGICK
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += conv-magick.la
conv_magick_la_SOURCES = conv-magick/from-magick.cpp conv-magick/to-magick.cpp
conv_magick_la_LIBADD = $(libmagick_LIBS)
else
libbuiltin_la_SOURCES += conv-magick/from-magick.cpp conv-magick/to-magick.cpp
libbuiltin_la_LIBADD += $(libmagick_LIBS)
endif
endif

if WITH_MAT
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += conv-mat.la
conv_mat_la_SOURCES = conv-mat/from-mat.cpp conv-mat/to-mat.cpp
conv_mat_la_LIBADD = $(libmatio_LIBS)
else
libbuiltin_la_SOURCES += conv-mat/from-mat.cpp conv-mat/to-mat.cpp
libbuiltin_la_LIBADD += $(libmatio_LIBS)
endif
endif

if WITH_NETCDF
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += conv-netcdf.la
conv_netcdf_la_SOURCES = conv-netcdf/from-netcdf.cpp conv-netcdf/to-netcdf.cpp
conv_netcdf_la_LIBADD = $(libnetcdf_LIBS)
else
libbuiltin_la_SOURCES += conv-netcdf/from-netcdf.cpp conv-netcdf/to-netcdf.cpp
libbuiltin_la_LIBADD += $(libnetcdf_LIBS)
endif
endif

if WITH_NETPBM
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += conv-netpbm.la
conv_netpbm_la_SOURCES = conv-netpbm/from-netpbm.cpp conv-netpbm/to-netpbm.cpp
conv_netpbm_la_LIBADD = $(LTLIBNETPBM)
else
libbuiltin_la_SOURCES += conv-netpbm/from-netpbm.cpp conv-netpbm/to-netpbm.cpp
libbuiltin_la_LIBADD += $(LTLIBNETPBM)
endif
endif

if WITH_PCD
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += conv-pcd.la
conv_pcd_la_SOURCES = conv-pcd/from-pcd.cpp conv-pcd/to-pcd.cpp
conv_pcd_la_LIBADD = $(libpcl_io_LIBS)
else
libbuiltin_la_SOURCES += conv-pcd/from-pcd.cpp conv-pcd/to-pcd.cpp
libbuiltin_la_LIBADD += $(libpcl_io_LIBS)
endif
endif

if WITH_PFS
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += conv-pfs.la
conv_pfs_la_SOURCES = conv-pfs/from-pfs.cpp conv-pfs/to-pfs.cpp
conv_pfs_la_LIBADD = $(libpfs_LIBS)
else
libbuiltin_la_SOURCES += conv-pfs/from-pfs.cpp conv-pfs/to-pfs.cpp
libbuiltin_la_LIBADD += $(libpfs_LIBS)
endif
endif

if WITH_PLY
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += conv-ply.la
conv_ply_la_SOURCES = conv-ply/from-ply.cpp conv-ply/to-ply.cpp conv-ply/ply.h conv-ply/plyfile.cpp
else
libbuiltin_la_SOURCES += conv-ply/from-ply.cpp conv-ply/to-ply.cpp conv-ply/ply.h conv-ply/plyfile.cpp
endif
endif

if WITH_PMD
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += conv-pmd.la
conv_pmd_la_SOURCES = conv-pmd/from-pmd.cpp
else
libbuiltin_la_SOURCES += conv-pmd/from-pmd.cpp
endif
endif

if WITH_PVM
EXTRA_DIST += conv-pvm/README
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += conv-pvm.la
conv_pvm_la_SOURCES = conv-pvm/from-pvm.cpp conv-pvm/to-pvm.cpp conv-pvm/codebase.h conv-pvm/ddsbase.h conv-pvm/ddsbase.cpp
else
libbuiltin_la_SOURCES += conv-pvm/from-pvm.cpp conv-pvm/to-pvm.cpp conv-pvm/codebase.h conv-pvm/ddsbase.h conv-pvm/ddsbase.cpp
endif
endif

if WITH_RAT
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += conv-rat.la
conv_rat_la_SOURCES = conv-rat/from-rat.cpp conv-rat/to-rat.cpp
else
libbuiltin_la_SOURCES += conv-rat/from-rat.cpp conv-rat/to-rat.cpp
endif
endif

if WITH_RAW
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += conv-raw.la
conv_raw_la_SOURCES = conv-raw/from-raw.cpp conv-raw/to-raw.cpp
else
libbuiltin_la_SOURCES += conv-raw/from-raw.cpp conv-raw/to-raw.cpp
endif
endif

if WITH_SNDFILE
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += conv-sndfile.la
conv_sndfile_la_SOURCES = conv-sndfile/from-sndfile.cpp conv-sndfile/to-sndfile.cpp
conv_sndfile_la_LIBADD = $(libsndfile_LIBS)
else
libbuiltin_la_SOURCES += conv-sndfile/from-sndfile.cpp conv-sndfile/to-sndfile.cpp
libbuiltin_la_LIBADD += $(libsndfile_LIBS)
endif
endif

if WITH_TEEM
if DYNAMIC_MODULES
pkglib_LTLIBRARIES += conv-teem.la
conv_teem_la_SOURCES = conv-teem/from-teem.cpp conv-teem/to-teem.cpp
conv_teem_la_LIBADD = $(LTLIBTEEM)
else
libbuiltin_la_SOURCES += conv-teem/from-teem.cpp conv-teem/to-teem.cpp
libbuiltin_la_LIBADD += $(LTLIBTEEM)
endif
endif
