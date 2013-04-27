/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2013
 * Martin Lambers <marlam@marlam.de>
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

#include "base/msg.h"
#include "base/opt.h"

#include "conv.h"


extern "C" void gtatool_from_help(void)
{
    msg::req_txt("from <input-file> [<output-file>]\n"
            "\n"
            "Convert any type of input file to GTAs.\n"
            "This command tries to automatically find a suitable import filter "
            "based on the filename extension.\n"
            "This may fail; in that case please use one of the specific from-* "
            "commands manually.\n"
            "Example: from file.foo file.gta");
}

extern "C" int gtatool_from(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, 1, 2, arguments)) {
        return 1;
    }
    if (help.value()) {
        gtatool_from_help();
        return 0;
    }

    return conv(true, arguments, argc, argv);
}
