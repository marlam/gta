/*
 * from-dcmtk.cpp
 *
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2010  Martin Lambers <marlam@marlam.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <string>

#include <dcmtk/dcmimgle/dcmimage.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcrledrg.h>
#include <dcmtk/dcmjpeg/djdecode.h>

#include <gta/gta.hpp>

#include "msg.h"
#include "cio.h"
#include "opt.h"
#include "debug.h"


extern "C" void gtatool_from_dcmtk_help(void)
{
    msg::req_txt("from-dcmtk <input-file> [<output-file>]\n"
            "\n"
            "Converts DICOM files to GTAs using DCMTK.");
}

extern "C" int gtatool_from_dcmtk(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, 1, 2, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_from_dcmtk_help();
        return 0;
    }

    FILE *fo = stdout;
    std::string ifilename(arguments[0]);
    std::string ofilename("standard output");
    try
    {
        if (arguments.size() == 2)
        {
            ofilename = arguments[1];
            fo = cio::open(ofilename, "w");
        }
        if (cio::isatty(fo))
        {
            throw exc("refusing to write to a tty");
        }
    }
    catch (std::exception &e)
    {
        msg::err("%s", e.what());
        return 1;
    }

    try
    {
        // Open DICOM file
        DcmRLEDecoderRegistration::registerCodecs(OFFalse, OFFalse);
        DJDecoderRegistration::registerCodecs(EDC_photometricInterpretation, EUC_default, EPC_default, OFFalse);
        DcmFileFormat *dfile = new DcmFileFormat();
        OFCondition cond = dfile->loadFile(ifilename.c_str(), EXS_Unknown, EGL_withoutGL, DCM_MaxReadLength, ERM_autoDetect);
        if (cond.bad())
        {
            throw exc("Cannot import " + ifilename, cond.text());
        }
        E_TransferSyntax xfer = dfile->getDataset()->getOriginalXfer();
        DicomImage *di = new DicomImage(dfile, xfer, CIF_MayDetachPixelData | CIF_TakeOverExternalDataset);
        if (!di)
        {
            throw exc("Cannot import " + ifilename, ENOMEM);
        }
        if (di->getStatus() != EIS_Normal)
        {
            throw exc("Cannot import " + ifilename, DicomImage::getString(di->getStatus()));
        }
        di->hideAllOverlays();

        for (unsigned long frame = 0; frame < di->getFrameCount(); frame++)
        {
            // Create GTA
            gta::header hdr;
            hdr.set_dimensions(di->getWidth(), di->getHeight());
            hdr.dimension_taglist(0).set("INTERPRETATION", "X");
            hdr.dimension_taglist(1).set("INTERPRETATION", "Y");
            int bits;
            gta::type type;
            if (di->getDepth() <= 8)
            {
                bits = 8;
                type = gta::uint8;
            }
            else if (di->getDepth() <= 16)
            {
                bits = 16;
                type = gta::uint16;
            }
            else if (di->getDepth() <= 32)
            {
                bits = 32;
                type = gta::uint32;
            }
            else if (di->getDepth() <= 64)
            {
                bits = 64;
                type = gta::uint64;
            }
            else if (di->getDepth() <= 128)
            {
                bits = 128;
                type = gta::uint128;
            }
            else
            {
                throw exc("Cannot import " + ifilename, "unsupported depth value " + str::str(di->getDepth()));
            }
            if (di->isMonochrome())
            {
                hdr.set_components(type);
            }
            else
            {
                hdr.set_components(type, type, type);
            }

            // Set meta information
            {
                hdr.global_taglist().set("DICOM/FILENAME", ifilename.c_str());
                hdr.global_taglist().set("DICOM/FRAMES", str::str(di->getFrameCount()).c_str());
                hdr.global_taglist().set("DICOM/FRAME", str::str(frame).c_str());
                hdr.global_taglist().set("DICOM/TRANSFER_SYNTAX", DcmXfer(xfer).getXferName());
                const char *color_model = di->getString(di->getPhotometricInterpretation());
                if (color_model)
                {
                    hdr.global_taglist().set("DICOM/COLOR_MODEL", color_model);
                }
                hdr.global_taglist().set("DICOM/PIXEL_ASPECT_RATIO", str::str(di->getHeightWidthRatio()).c_str());
                hdr.global_taglist().set("DICOM/BITS_PER_SAMPLE", str::str(di->getDepth()).c_str());
            }

            // Extract data
            const void *data = di->getOutputData(bits, frame, 0);
            if (!data)
            {
                throw exc("Cannot import " + ifilename, "failed to render frame " + str::str(frame));
            }

            // Save GTA
            hdr.write_to(fo);
            hdr.write_data(fo, data);
        }
        // Cleanup
        DcmRLEDecoderRegistration::cleanup();
        DJDecoderRegistration::cleanup();
        if (fo != stdout)
        {
            cio::close(fo);
        }
    }
    catch (std::exception &e)
    {
        msg::err("%s", e.what());
        return 1;
    }
    catch (...)
    {
        msg::err("DCMTK error");
        return 1;
    }

    return 0;
}