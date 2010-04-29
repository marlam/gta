/*
 * from-pfs.cpp
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

#include <pfs-1.2/pfs.h>

#include <gta/gta.hpp>

#include "msg.h"
#include "blob.h"
#include "cio.h"
#include "opt.h"
#include "debug.h"
#include "intcheck.h"


extern "C" void gtatool_from_pfs_help(void)
{
    msg::req_txt("from-pfs <input-file> [<output-file>]\n"
            "\n"
            "Converts PFS files to GTAs using libpfs.");
}

extern "C" int gtatool_from_pfs(int argc, char *argv[])
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
        gtatool_from_pfs_help();
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
        FILE *fi = cio::open(ifilename, "r");
        for (;;)
        {
            pfs::DOMIO pfsio;
            pfs::Frame *frame = pfsio.readFrame(fi);
            if (!frame)
            {
                break;
            }
            gta::header hdr;
            pfs::TagIteratorPtr tit(frame->getTags()->getIterator());
            while (tit->hasNext())
            {
                const char *tag_name = tit->getNext();
                if (strncmp(tag_name, "X-GTA/", 6) == 0)
                {
                    hdr.global_taglist().set(tag_name + 6,
                            frame->getTags()->getString(tag_name));
                }
                else
                {
                    hdr.global_taglist().set((std::string("PFS/") + tag_name).c_str(),
                            frame->getTags()->getString(tag_name));
                }
            }
            hdr.set_dimensions(frame->getWidth(), frame->getHeight());
            hdr.dimension_taglist(0).set("INTERPRETATION", "X");
            hdr.dimension_taglist(1).set("INTERPRETATION", "Y");
            uintmax_t components = 0;
            pfs::ChannelIteratorPtr cit(frame->getChannelIterator());
            while (cit->hasNext())
            {
                cit->getNext();
                components++;
            }
            std::vector<gta::type> types(components);
            for (uintmax_t i = 0; i < components; i++)
            {
                types[i] = gta::float32;
            }
            hdr.set_components(components, &(types[0]));
            std::vector<float *> channels(components);
            pfs::ChannelIteratorPtr cit2(frame->getChannelIterator());
            uintmax_t i = 0;
            while (cit2->hasNext())
            {
                pfs::Channel *channel = cit2->getNext();
                if (strcmp(channel->getName(), "X") == 0)
                {
                    hdr.component_taglist(i).set("INTERPRETATION", "XYZ/X");
                }
                else if (strcmp(channel->getName(), "Y") == 0)
                {
                    hdr.component_taglist(i).set("INTERPRETATION", "XYZ/Y");
                }
                else if (strcmp(channel->getName(), "Z") == 0)
                {
                    hdr.component_taglist(i).set("INTERPRETATION", "XYZ/Z");
                }
                else if (strcmp(channel->getName(), "ALPHA") == 0)
                {
                    hdr.component_taglist(i).set("INTERPRETATION", "ALPHA");
                }
                else if (strncmp(channel->getName(), "X-GTA/", 6) == 0)
                {
                    hdr.component_taglist(i).set("INTERPRETATION", channel->getName() + 6);
                }
                else
                {
                    hdr.component_taglist(i).set("PFS/NAME", channel->getName());
                }
                pfs::TagIteratorPtr tit2(channel->getTags()->getIterator());
                while (tit2->hasNext())
                {
                    const char *tag_name = tit2->getNext();
                    if (strncmp(tag_name, "X-GTA/", 6) == 0)
                    {
                        hdr.global_taglist().set(tag_name + 6,
                                channel->getTags()->getString(tag_name));
                    }
                    else
                    {
                        hdr.component_taglist(i).set((std::string("PFS/") + tag_name).c_str(),
                                channel->getTags()->getString(tag_name));
                    }
                }
                channels[i] = channel->getRawData();
                i++;
            }
            blob data(sizeof(float), checked_cast<size_t>(hdr.dimension_size(0)),
                    checked_cast<size_t>(hdr.dimension_size(1)),
                    checked_cast<size_t>(hdr.components()));
            for (uintmax_t y = 0; y < hdr.dimension_size(1); y++)
            {
                for (uintmax_t x = 0; x < hdr.dimension_size(0); x++)
                {
                    for (uintmax_t i = 0; i < components; i++)
                    {
                        *data.ptr<float>((y * hdr.dimension_size(0) + x) * hdr.components() + i) =
                            channels[i][y * hdr.dimension_size(0) + x];
                    }
                }
            }
            pfsio.freeFrame(frame);
            hdr.write_to(fo);
            hdr.write_data(fo, data.ptr());
        }
        if (fo != stdout)
        {
            cio::close(fo);
        }
    }
    catch (pfs::Exception &e)
    {
        msg::err("%s", e.getMessage());
        return 1;
    }
    catch (std::exception &e)
    {
        msg::err("%s", e.what());
        return 1;
    }

    return 0;
}
