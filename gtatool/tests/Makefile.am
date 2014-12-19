# Copyright (C) 2010, 2011, 2012, 2013, 2014
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

EXTRA_DIST = \
	gta-help.sh \
	gta-version.sh \
	gta-component-add.sh \
	gta-component-set.sh \
	gta-component-convert.sh \
	gta-component-extract.sh \
	gta-component-merge.sh \
	gta-component-reorder.sh \
	gta-component-split.sh \
	gta-dimension-add.sh \
	gta-dimension-extract.sh \
	gta-dimension-flatten.sh \
	gta-dimension-merge.sh \
	gta-dimension-reorder.sh \
	gta-dimension-reverse.sh \
	gta-dimension-split.sh \
	gta-combine.sh \
	gta-compress.sh \
	gta-create.sh \
	gta-diff.sh \
	gta-extract.sh \
	gta-fill.sh \
	gta-info.sh \
	gta-merge.sh \
	gta-resize.sh \
	gta-set.sh \
	gta-tag.sh \
	gta-uncompress.sh \
	gta-stream-extract.sh \
	gta-stream-foreach.sh \
	gta-stream-grep.sh \
	gta-stream-merge.sh \
	gta-stream-split.sh \
	gta-component-compute.sh \
	gta-gui.sh \
	gta-view.sh \
	conv-csv.sh \
	conv-datraw.sh \
	conv-dcmtk.sh \
	conv-exr.sh \
	conv-ffmpeg.sh \
	conv-gdal.sh \
	conv-jpeg.sh \
	conv-magick.sh \
	conv-mat.sh \
	conv-netcdf.sh \
	conv-netpbm.sh \
	conv-pcd.sh \
	conv-pfs.sh \
	conv-ply.sh \
	conv-pmd.sh \
	conv-png.sh \
	conv-pvm.sh \
	conv-rat.sh \
	conv-raw.sh \
	conv-sndfile.sh \
	conv-teem.sh

TESTS = \
	gta-help.sh \
	gta-version.sh \
	gta-component-add.sh \
	gta-component-set.sh \
	gta-component-convert.sh \
	gta-component-extract.sh \
	gta-component-merge.sh \
	gta-component-reorder.sh \
	gta-component-split.sh \
	gta-dimension-add.sh \
	gta-dimension-extract.sh \
	gta-dimension-flatten.sh \
	gta-dimension-merge.sh \
	gta-dimension-reorder.sh \
	gta-dimension-reverse.sh \
	gta-dimension-split.sh \
	gta-combine.sh \
	gta-compress.sh \
	gta-create.sh \
	gta-diff.sh \
	gta-extract.sh \
	gta-fill.sh \
	gta-info.sh \
	gta-merge.sh \
	gta-resize.sh \
	gta-set.sh \
	gta-tag.sh \
	gta-uncompress.sh \
	gta-stream-extract.sh \
	gta-stream-foreach.sh \
	gta-stream-grep.sh \
	gta-stream-merge.sh \
	gta-stream-split.sh

if WITH_MUPARSER
TESTS += gta-component-compute.sh
endif
if WITH_QT
TESTS += gta-gui.sh
if WITH_GLEWMX
TESTS += gta-view.sh
endif
endif

if WITH_CSV
TESTS += conv-csv.sh
endif
if WITH_DATRAW
TESTS += conv-datraw.sh
endif
if WITH_DCMTK
TESTS += conv-dcmtk.sh
endif
if WITH_EXR
TESTS += conv-exr.sh
endif
if WITH_FFMPEG
TESTS += conv-ffmpeg.sh
endif
if WITH_GDAL
TESTS += conv-gdal.sh
endif
if WITH_JPEG
TESTS += conv-jpeg.sh
endif
if WITH_MAGICK
TESTS += conv-magick.sh
endif
if WITH_MAT
TESTS += conv-mat.sh
endif
if WITH_NETCDF
TESTS += conv-netcdf.sh
endif
if WITH_NETPBM
TESTS += conv-netpbm.sh
endif
if WITH_PCD
TESTS += conv-pcd.sh
endif
if WITH_PFS
TESTS += conv-pfs.sh
endif
if WITH_PLY
TESTS += conv-ply.sh
endif
if WITH_PMD
TESTS += conv-pmd.sh
endif
if WITH_PNG
TESTS += conv-png.sh
endif
if WITH_PVM
TESTS += conv-pvm.sh
endif
if WITH_RAT
TESTS += conv-rat.sh
endif
if WITH_RAW
TESTS += conv-raw.sh
endif
if WITH_SNDFILE
TESTS += conv-sndfile.sh
endif
if WITH_TEEM
TESTS += conv-teem.sh
endif

if VALGRIND_TESTS
VALGRIND_CMD = $(VALGRIND) --quiet --log-fd=2 --error-exitcode=1 \
	--tool=memcheck --leak-check=full --leak-resolution=high --show-reachable=no
else
VALGRIND_CMD =
endif

TESTS_ENVIRONMENT = \
	GTA="$(VALGRIND_CMD) $(top_builddir)/src/gta" \
	GTATOOL_PLUGIN_PATH="$(top_builddir)/src/.libs"
